#pragma once

#include "rs-core/global.hpp"
#include "rs-core/iterator.hpp"
#include "rs-core/linear-algebra.hpp"
#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

namespace RS::Containers {

    template <std::regular T, int N>
    class MultiArray {

    private:

        static_assert(N >= 1);

        template <typename CMA, typename CT>
        class basic_iterator:
        public Iterator<basic_iterator<CMA, CT>, CT, std::bidirectional_iterator_tag> {

        public:

            basic_iterator() = default;
            basic_iterator(const basic_iterator<std::remove_const_t<CMA>, std::remove_const_t<CT>>& i):
                owner_(i.owner_), data_(i.data_), index_(i.index_) {}

            CT& operator*() const noexcept { return data_[index_]; }
            basic_iterator& operator++() noexcept { ++index_; return *this; }
            basic_iterator& operator--() noexcept { --index_; return *this; }
            bool operator==(const basic_iterator& i) const noexcept { return index_ == i.index_; }

            basic_iterator& move(int axis, int distance = 1) noexcept {
                index_ += owner_->factors_[static_cast<std::size_t>(axis)] * distance;
                return *this;
            }

            Vector<int, N> pos() const noexcept { return owner_->index_to_position(index_); }

        private:

            friend class MultiArray;

            CMA* owner_;
            CT* data_;
            int index_;

            basic_iterator(CMA& owner, int index) noexcept:
                owner_(&owner), data_(owner.store_.data()), index_(index) {}

        };

    public:

        using iterator = basic_iterator<MultiArray, T>;
        using const_iterator = basic_iterator<const MultiArray, const T>;
        using position = Vector<int, N>;
        using value_type = T;

        static constexpr int dim = N;

        MultiArray() = default;
        explicit MultiArray(const position& shape) { do_reset(shape); }
        explicit MultiArray(const position& shape, const T& t) { do_reset(shape, t); }
        template <std::convertible_to<int>... Args>
            explicit MultiArray(Args... args)
            requires (sizeof...(Args) == N):
            MultiArray{position(args...)} {}
        MultiArray(const MultiArray& a) = default;
        MultiArray(MultiArray&& a) = default;
        ~MultiArray() = default;
        MultiArray& operator=(const MultiArray& a) = default;
        MultiArray& operator=(MultiArray&& a) = default;

        T& operator[](const position& p) noexcept { return ref(p); }
        const T& operator[](const position& p) const noexcept { return get(p); }
        template <std::convertible_to<int>... Args> T& operator[](Args... args) noexcept
            requires (sizeof...(Args) == N);
        template <std::convertible_to<int>... Args> const T& operator[](Args... args) const noexcept
            requires (sizeof...(Args) == N);

        iterator begin() noexcept { return iterator{*this, 0}; }
        const_iterator begin() const noexcept { return const_iterator{*this, 0}; }
        iterator end() noexcept { return iterator{*this, int(size())}; }
        const_iterator end() const noexcept { return const_iterator{*this, int(size())}; }
        T* data() noexcept { return store_.data(); }
        const T* data() const noexcept { return store_.data(); }

        iterator locate(const position& p) noexcept;
        const_iterator locate(const position& p) const noexcept;
        template <std::convertible_to<int>... Args> iterator locate(Args... args) noexcept
            requires (sizeof...(Args) == N);
        template <std::convertible_to<int>... Args> const_iterator locate(Args... args) const noexcept
            requires (sizeof...(Args) == N);
        T& ref(const position& p) noexcept;
        template <std::convertible_to<int>... Args> T& ref(Args... args) noexcept
            requires (sizeof...(Args) == N);
        const T& get(const position& p) const noexcept;
        template <std::convertible_to<int>... Args> const T& get(Args... args) const noexcept
            requires (sizeof...(Args) == N);

        bool contains(const position& p) const noexcept;
        template <std::convertible_to<int>... Args> bool contains(Args... args) const noexcept
            requires (sizeof...(Args) == N);
        bool empty() const noexcept { return size() == 0; }
        position shape() const noexcept { return shape_; }
        std::size_t size() const noexcept { return static_cast<std::size_t>(factors_[N]); }

        void clear() noexcept { shape_ = position(0); store_.clear(); }
        void fill(const T& t) { std::fill(begin(), end(), t); }

        void reset(const position& shape) { do_reset(shape); }
        void reset(const position& shape, const T& t) { do_reset(shape, t); }
        template <std::convertible_to<int>... Args> void reset(Args... args)
            requires (sizeof...(Args) == N);

        void swap(MultiArray& a) noexcept;
        friend void swap(MultiArray& a, MultiArray& b) noexcept { a.swap(b); }

        friend bool operator==(const MultiArray& a, const MultiArray& b) noexcept {
            return a.shape_ == b.shape_ && std::equal(a.begin(), a.end(), b.begin(), b.end());
        }

    private:

        std::vector<T> store_;
        position shape_ {0};
        Vector<int, N + 1> factors_;

        void do_reset(const position& shape, const T& t = {});
        position index_to_position(int i) const noexcept;
        int position_to_index(const position& p) const noexcept;

    };

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        T& MultiArray<T, N>::operator[](Args... args) noexcept
        requires (sizeof...(Args) == N) {
            return ref(args...);
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        const T& MultiArray<T, N>::operator[](Args... args) const noexcept
        requires (sizeof...(Args) == N) {
            return get(args...);
        }

        template <std::regular T, int N>
        typename MultiArray<T, N>::iterator
        MultiArray<T, N>::locate(const position& p) noexcept {
            return iterator{*this, position_to_index(p)};
        }

        template <std::regular T, int N>
        typename MultiArray<T, N>::const_iterator
        MultiArray<T, N>::locate(const position& p) const noexcept {
            return const_iterator{*this, position_to_index(p)};
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        typename MultiArray<T, N>::iterator
        MultiArray<T, N>::locate(Args... args) noexcept
        requires (sizeof...(Args) == N) {
            return iterator{*this, position_to_index({args...})};
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        typename MultiArray<T, N>::const_iterator
        MultiArray<T, N>::locate(Args... args) const noexcept
        requires (sizeof...(Args) == N) {
            return const_iterator{*this, position_to_index({args...})};
        }

        template <std::regular T, int N>
        T& MultiArray<T, N>::ref(const position& p) noexcept {
            return store_[static_cast<std::size_t>(position_to_index(p))];
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        T& MultiArray<T, N>::ref(Args... args) noexcept
        requires (sizeof...(Args) == N) {
            return store_[static_cast<std::size_t>(position_to_index({args...}))];
        }

        template <std::regular T, int N>
        const T& MultiArray<T, N>::get(const position& p) const noexcept {
            return store_[static_cast<std::size_t>(position_to_index(p))];
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        const T& MultiArray<T, N>::get(Args... args) const noexcept
        requires (sizeof...(Args) == N) {
            return store_[static_cast<std::size_t>(position_to_index({args...}))];
        }

        template <std::regular T, int N>
        bool MultiArray<T, N>::contains(const position& p) const noexcept {
            for (auto i = 0uz; i < static_cast<std::size_t>(N); ++i) {
                if (p[i] < 0 || p[i] >= shape_[i]) {
                    return false;
                }
            }
            return true;
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        bool MultiArray<T, N>::contains(Args... args) const noexcept
        requires (sizeof...(Args) == N) {
            return contains({args...});
        }

        template <std::regular T, int N>
        template <std::convertible_to<int>... Args>
        void MultiArray<T, N>::reset(Args... args)
        requires (sizeof...(Args) == N) {
            do_reset(position(args...));
        }

        template <std::regular T, int N>
        void MultiArray<T, N>::swap(MultiArray& a) noexcept {
            std::swap(store_, a.store_);
            std::swap(shape_, a.shape_);
            std::swap(factors_, a.factors_);
        }

        template <std::regular T, int N>
        void MultiArray<T, N>::do_reset(const position& shape, const T& t) {
            auto old_size = factors_[N];
            shape_ = shape;
            factors_[0] = 1;
            for (auto i = 0uz; i < static_cast<std::size_t>(N); ++i) {
                factors_[i + 1] = factors_[i] * shape_[i];
            }
            auto new_size = factors_[N];
            store_.resize(std::size_t(new_size), t);
            std::fill(store_.begin(), store_.begin() + std::min(old_size, new_size), t);
        }

        template <std::regular T, int N>
        typename MultiArray<T, N>::position
        MultiArray<T, N>::index_to_position(int i) const noexcept {
            position p{0};
            if (! empty()) {
                for (auto j = static_cast<std::size_t>(N); j > 0; --j) {
                    p[j - 1] = i / factors_[j - 1];
                    i %= factors_[j - 1];
                }
            }
            return p;
        }

        template <std::regular T, int N>
        int MultiArray<T, N>::position_to_index(const position& p) const noexcept {
            return std::inner_product(p.begin(), p.end(), factors_.begin(), 0);
        }

}
