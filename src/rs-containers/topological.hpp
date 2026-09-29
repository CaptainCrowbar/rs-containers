#pragma once

#include "rs-core/global.hpp"
#include "rs-core/iterator.hpp"
#include "rs-core/range.hpp"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace RS::Containers {

    template <std::regular T, std::strict_weak_order<T, T> Cmp = std::less<T>>
    class TopologicalOrder;

    namespace Detail {

        // Inspectors for debugging (implemented in the test file)
        template <std::regular T, std::strict_weak_order<T, T> Cmp>
            std::string format_by_node(TopologicalOrder<T, Cmp> topo);
        template <std::regular T, std::strict_weak_order<T, T> Cmp>
            std::string format_by_set(TopologicalOrder<T, Cmp> topo);

    }

    class TopologicalCycle:
    public std::runtime_error {
    public:
        TopologicalCycle():
            std::runtime_error{"Dependency cycle in topological sort"} {}
    };

    template <std::regular T, std::strict_weak_order<T, T> Cmp>
    class TopologicalOrder {

    public:

        using value_type = T;
        using compare_type = Cmp;

        TopologicalOrder() = default;
        explicit TopologicalOrder(Cmp c): graph_{c} {}

        Cmp comp() const { return graph_.key_comp(); }
        bool contains(const T& t) const { return graph_.count(t) != 0; }
        bool empty() const noexcept { return graph_.empty(); }
        std::size_t size() const noexcept { return graph_.size(); }

        void insert(const T& t) { ensure_key_exists(t); }
        template <std::convertible_to<T>... Args> void insert(const Args&... args);
        template <ReadableRange<T> Range> void insert(const Range& r);
        template <ReadableForwardRange<T> Range> void insert(const T& t1, const Range& r2);
        void insert(const T& t1, std::initializer_list<T> r2);
        template <ReadableForwardRange<T> Range> void insert(const Range& r1, const T& t2);
        void insert(std::initializer_list<T> r1, const T& t2);
        template <ReadableForwardRange<T> Range1, ReadableForwardRange<T> Range2>
            void insert(const Range1& r1, const Range2& r2);
        void insert(std::initializer_list<T> r1, std::initializer_list<T> r2);
        bool erase(const T& t);
        bool erase_edge(const T& t1, const T& t2);
        std::size_t erase_orphans();
        void clear() noexcept { graph_.clear(); }

        std::vector<T> front_set() const;
        std::vector<T> back_set() const;
        std::optional<T> front() const;
        std::optional<T> back() const;
        std::vector<T> pop_front_set();
        std::vector<T> pop_back_set();
        std::optional<T> pop_front();
        std::optional<T> pop_back();
        bool is_front(const T& t) const;
        bool is_back(const T& t) const;

    private:

        friend std::string Detail::format_by_node<>(TopologicalOrder topo);
        friend std::string Detail::format_by_set<>(TopologicalOrder topo);

        using set_type = std::set<T, Cmp>;

        struct link_sets {
            set_type left;
            set_type right;
        };

        using node_links_map = std::map<T, link_sets, Cmp>;
        using node_links_iterator = typename node_links_map::iterator;

        node_links_map graph_;

        node_links_iterator ensure_key_exists(const T& t);
        template <ReadableForwardRange<T> Range1, ReadableForwardRange<T> Range2>
            void insert_ranges(const Range1& r1, const Range2& r2);
        void insert_vector(const std::vector<T>& v);
        link_sets make_link_sets() const;

    };

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <std::convertible_to<T>... Args>
        void TopologicalOrder<T, Cmp>::insert(const Args&... args) {
            std::vector<T> v{args...};
            insert_vector(v);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <ReadableRange<T> Range>
        void TopologicalOrder<T, Cmp>::insert(const Range& r) {
            std::vector<T> v(std::from_range, r);
            insert_vector(v);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <ReadableForwardRange<T> Range>
        void TopologicalOrder<T, Cmp>::insert(const T& t1, const Range& r2) {
            insert_ranges(std::views::single(t1), r2);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        void TopologicalOrder<T, Cmp>::insert(const T& t1, std::initializer_list<T> r2) {
            insert_ranges(std::views::single(t1), r2);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <ReadableForwardRange<T> Range>
        void TopologicalOrder<T, Cmp>::insert(const Range& r1, const T& t2) {
            insert_ranges(r1, std::views::single(t2));
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        void TopologicalOrder<T, Cmp>::insert(std::initializer_list<T> r1, const T& t2) {
            insert_ranges(r1, std::views::single(t2));
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <ReadableForwardRange<T> Range1, ReadableForwardRange<T> Range2>
        void TopologicalOrder<T, Cmp>::insert(const Range1& r1, const Range2& r2) {
            insert_ranges(r1, r2);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        void TopologicalOrder<T, Cmp>::insert(std::initializer_list<T> r1, std::initializer_list<T> r2) {
            insert_ranges(r1, r2);
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        bool TopologicalOrder<T, Cmp>::erase(const T& t) {

            auto i = graph_.find(t);

            if (i == graph_.end()) {
                return false;
            }

            graph_.erase(i);

            for (auto& node: graph_) {
                node.second.left.erase(t);
                node.second.right.erase(t);
            }

            return true;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        bool TopologicalOrder<T, Cmp>::erase_edge(const T& t1, const T& t2) {

            auto i = graph_.find(t1);

            if (i == graph_.end()) {
                return false;
            }

            auto j = graph_.find(t2);

            if (j == graph_.end()) {
                return false;
            }

            auto n = i->second.left.erase(t2)
                + i->second.right.erase(t2)
                + j->second.left.erase(t1)
                + j->second.right.erase(t1);

            return n > 0;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::size_t TopologicalOrder<T, Cmp>::TopologicalOrder::erase_orphans() {
            return std::erase_if(graph_, [] (const auto& node) {
                return node.second.left.empty() && node.second.right.empty();
            });
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::vector<T> TopologicalOrder<T, Cmp>::front_set() const {

            std::vector<T> v;

            if (graph_.empty()) {
                return v;
            }

            for (auto& node: graph_) {
                if (node.second.left.empty()) {
                    v.push_back(node.first);
                }
            }

            if (v.empty()) {
                throw TopologicalCycle{};
            }

            return v;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::vector<T> TopologicalOrder<T, Cmp>::back_set() const {

            std::vector<T> v;

            if (graph_.empty()) {
                return v;
            }

            for (auto& node: graph_) {
                if (node.second.right.empty()) {
                    v.push_back(node.first);
                }
            }

            if (v.empty()) {
                throw TopologicalCycle{};
            }

            return v;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::optional<T> TopologicalOrder<T, Cmp>::front() const {

            if (graph_.empty()) {
                return {};
            }

            for (auto& node: graph_) {
                if (node.second.left.empty()) {
                    return node.first;
                }
            }

            throw TopologicalCycle{};

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::optional<T> TopologicalOrder<T, Cmp>::back() const {

            if (graph_.empty()) {
                return {};
            }

            for (auto& node: graph_) {
                if (node.second.right.empty()) {
                    return node.first;
                }
            }

            throw TopologicalCycle{};

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::vector<T> TopologicalOrder<T, Cmp>::pop_front_set() {

            std::vector<T> v;

            if (graph_.empty()) {
                return v;
            }

            auto i = graph_.begin();
            auto g_end = graph_.end();

            while (i != g_end) {
                if (i->second.left.empty()) {
                    v.push_back(i->first);
                    graph_.erase(i++);
                } else {
                    ++i;
                }
            }

            if (v.empty()) {
                throw TopologicalCycle{};
            }

            for (auto& node: graph_) {
                auto remains = make_link_sets();
                std::ranges::set_difference(node.second.left, v,
                    std::inserter(remains.left, remains.left.end()), graph_.key_comp());
                std::ranges::set_difference(node.second.right, v,
                    std::inserter(remains.right, remains.right.end()), graph_.key_comp());
                node.second = std::move(remains);
            }

            return v;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::vector<T> TopologicalOrder<T, Cmp>::pop_back_set() {

            std::vector<T> v;

            if (graph_.empty()) {
                return v;
            }

            auto i = graph_.begin();
            auto g_end = graph_.end();

            while (i != g_end) {
                if (i->second.right.empty()) {
                    v.push_back(i->first);
                    graph_.erase(i++);
                } else {
                    ++i;
                }
            }

            if (v.empty()) {
                throw TopologicalCycle{};
            }

            for (auto& node: graph_) {
                auto remains = make_link_sets();
                std::ranges::set_difference(node.second.left, v,
                    std::inserter(remains.left, remains.left.end()), graph_.key_comp());
                std::ranges::set_difference(node.second.right, v,
                    std::inserter(remains.right, remains.right.end()), graph_.key_comp());
                node.second = std::move(remains);
            }

            return v;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::optional<T> TopologicalOrder<T, Cmp>::pop_front() {

            if (graph_.empty()) {
                return {};
            }

            auto i = std::ranges::find_if(graph_,
                [] (auto& node) { return node.second.left.empty(); });

            if (i == graph_.end()) {
                throw TopologicalCycle{};
            }

            auto t = i->first;
            graph_.erase(i);

            for (auto& node: graph_) {
                node.second.left.erase(t);
                node.second.right.erase(t);
            }

            return t;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        std::optional<T> TopologicalOrder<T, Cmp>::pop_back() {

            if (graph_.empty()) {
                return {};
            }

            auto i = std::find_if(graph_.begin(), graph_.end(),
                [] (auto& node) { return node.second.right.empty(); });

            if (i == graph_.end()) {
                throw TopologicalCycle{};
            }

            T t = i->first;
            graph_.erase(i);

            for (auto& node: graph_) {
                node.second.left.erase(t);
                node.second.right.erase(t);
            }

            return t;

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        bool TopologicalOrder<T, Cmp>::is_front(const T& t) const {
            auto i = graph_.find(t);
            return i != graph_.end() && i->second.left.empty();
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        bool TopologicalOrder<T, Cmp>::is_back(const T& t) const {
            auto i = graph_.find(t);
            return i != graph_.end() && i->second.right.empty();
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        typename TopologicalOrder<T, Cmp>::node_links_iterator
        TopologicalOrder<T, Cmp>::ensure_key_exists(const T& t) {
            auto i = graph_.find(t);
            if (i == graph_.end()) {
                i = graph_.insert({t, make_link_sets()}).first;
            }
            return i;
        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        template <ReadableForwardRange<T> Range1, ReadableForwardRange<T> Range2>
        void TopologicalOrder<T, Cmp>::insert_ranges(const Range1& r1, const Range2& r2) {

            for (auto& t1: r1) {
                auto i = ensure_key_exists(t1);
                std::ranges::copy(r2, std::inserter(i->second.right, i->second.right.end()));
                i->second.right.erase(t1);
            }

            for (auto& t2: r2) {
                auto j = ensure_key_exists(t2);
                std::ranges::copy(r1, std::inserter(j->second.left, j->second.left.end()));
                j->second.left.erase(t2);
            }

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        void TopologicalOrder<T, Cmp>::insert_vector(const std::vector<T>& v) {

            if (v.empty()) {
                return;
            }

            std::vector<node_links_iterator> its(v.size());

            for (auto i = 0uz; i < v.size(); ++i) {
                its[i] = ensure_key_exists(v[i]);
            }

            for (auto i = 0uz; i < v.size() - 1; ++i) {
                for (auto j = i + 1; j < v.size(); ++j) {
                    if (its[i] != its[j]) {
                        its[i]->second.right.insert(v[j]);
                        its[j]->second.left.insert(v[i]);
                    }
                }
            }

        }

        template <std::regular T, std::strict_weak_order<T, T> Cmp>
        typename TopologicalOrder<T, Cmp>::link_sets
        TopologicalOrder<T, Cmp>::make_link_sets() const {
            return {set_type(graph_.key_comp()), set_type(graph_.key_comp())};
        }

}
