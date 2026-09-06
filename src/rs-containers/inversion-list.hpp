#pragma once

#include "rs-core/format.hpp"
#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <initializer_list>
#include <ranges>
#include <tuple>
#include <vector>

namespace RS::Containers {

    template <std::integral Key>
    class InversionSet {

    public:

        using key_type = Key;

        struct block_type {
            Key first;
            Key last;
            block_type() = default;
            block_type(Key left, Key right) noexcept: first{left}, last{right} {}
            bool operator==(const block_type& b) const noexcept = default;
            auto operator<=>(const block_type& b) const noexcept = default;
            bool is_valid() const noexcept { return first <= last; }
        };

    private:

        using block_list = std::vector<block_type>;

        block_list blocks_;

    public:

        using iterator = typename block_list::const_iterator;
        using const_iterator = iterator;

        InversionSet() = default;
        InversionSet(std::initializer_list<block_type> list);

        iterator begin() const noexcept { return blocks_.begin(); }
        iterator end() const noexcept { return blocks_.end(); }
        bool empty() const noexcept { return blocks_.empty(); }
        std::size_t size() const noexcept { return blocks_.size(); }
        bool contains(Key key) const noexcept;
        bool operator()(Key key) const noexcept { return contains(key); }

    };

        template <std::integral Key>
        InversionSet<Key>::InversionSet(std::initializer_list<block_type> list):
        blocks_{list} {
            std::erase_if(blocks_, std::not_fn(&block_type::is_valid));
            std::ranges::sort(blocks_);
        }

        template <std::integral Key>
        bool InversionSet<Key>::contains(Key key) const noexcept {
            auto it = std::ranges::upper_bound(blocks_, key, std::less<Key>{}, &block_type::first);
            return it != blocks_.begin() && key <= (it - 1)->last;
        }

    template <std::integral Key, std::semiregular Value>
    class InversionMap {

    public:

        using key_type = Key;
        using mapped_type = Value;

        struct block_type {
            Key first;
            Key last;
            Value value;
            block_type() = default;
            block_type(Key left, Key right, const Value& val) noexcept: first{left}, last{right}, value{val} {}
            bool operator==(const block_type& b) const noexcept { return std::tie(first, last) == std::tie(b.first, b.last); }
            auto operator<=>(const block_type& b) const noexcept { return std::tie(first, last) <=> std::tie(b.first, b.last); }
            bool is_valid() const noexcept { return first <= last; }
        };

    private:

        using block_list = std::vector<block_type>;

        block_list blocks_;
        Value default_ {};

        void collate();

    public:

        using iterator = typename block_list::const_iterator;
        using const_iterator = iterator;

        InversionMap() = default;
        InversionMap(std::initializer_list<block_type> list);
        explicit InversionMap(std::initializer_list<block_type> list, const Value& default_value);

        const Value& default_value() const noexcept { return default_; }
        iterator begin() const noexcept { return blocks_.begin(); }
        iterator end() const noexcept { return blocks_.end(); }
        bool empty() const noexcept { return blocks_.empty(); }
        std::size_t size() const noexcept { return blocks_.size(); }
        bool contains(Key key) const noexcept;
        const Value& get(Key key) const noexcept;
        const Value& operator[](Key key) const noexcept { return get(key); }

    };

        template <std::integral Key, std::semiregular Value>
        InversionMap<Key, Value>::InversionMap(std::initializer_list<block_type> list):
        blocks_{list} {
            collate();
        }

        template <std::integral Key, std::semiregular Value>
        InversionMap<Key, Value>::InversionMap(std::initializer_list<block_type> list, const Value& default_value):
        blocks_{list},
        default_{default_value} {
            collate();
        }

        template <std::integral Key, std::semiregular Value>
        bool InversionMap<Key, Value>::contains(Key key) const noexcept {
            auto it = std::ranges::upper_bound(blocks_, key, std::less<Key>{}, &block_type::first);
            return it != blocks_.begin() && key <= (it - 1)->last;
        }

        template <std::integral Key, std::semiregular Value>
        const Value& InversionMap<Key, Value>::get(Key key) const noexcept {
            auto it = std::ranges::upper_bound(blocks_, key, std::less<Key>{}, &block_type::first);
            if (it != blocks_.begin() && key <= (it - 1)->last) {
                return (it - 1)->value;
            } else {
                return default_;
            }
        }

        template <std::integral Key, std::semiregular Value>
        void InversionMap<Key, Value>::collate() {
            std::erase_if(blocks_, std::not_fn(&block_type::is_valid));
            std::ranges::sort(blocks_);
        }

}

template <std::integral Key>
struct std::formatter<typename RS::Containers::InversionSet<Key>>:
RS::CommonFormatter {

    template <typename FormatContext>
    auto format(const RS::Containers::InversionSet<Key>& set, FormatContext& ctx) const {

        if (set.empty()) {

            write_out("{}", ctx.out());

        } else {

            std::formatter<Key> format_key;
            auto delimiter = '{';

            for (const auto& sub: set) {
                *ctx.out() = delimiter;
                format_key.format(sub.first, ctx);
                *ctx.out() = ':';
                format_key.format(sub.last, ctx);
                delimiter = ',';
            }

            *ctx.out() = '}';

        }

        return ctx.out();

    }

};

template <std::integral Key, std::semiregular Value>
struct std::formatter<typename RS::Containers::InversionMap<Key, Value>>:
RS::CommonFormatter {

    template <typename FormatContext>
    auto format(const RS::Containers::InversionMap<Key, Value>& set, FormatContext& ctx) const {

        std::formatter<Key> format_key;
        std::formatter<Value> format_value;

        if constexpr (RS::EscapeFormat<Value>) {
            format_value.set_debug_format();
        }

        *ctx.out() = '{';

        for (const auto& sub: set) {
            format_key.format(sub.first, ctx);
            *ctx.out() = ':';
            format_key.format(sub.last, ctx);
            *ctx.out() = ':';
            format_value.format(sub.value, ctx);
            *ctx.out() = ',';
        }

        write_out("default:", ctx.out());
        format_value.format(set.default_value(), ctx);
        *ctx.out() = '}';

        return ctx.out();

    }

};
