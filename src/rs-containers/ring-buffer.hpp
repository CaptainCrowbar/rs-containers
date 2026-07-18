#pragma once

#include "rs-core/arithmetic.hpp"
#include "rs-core/enum.hpp"
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

namespace RS::Containers {

    RS_BITMASK(RingPolicy, unsigned char,
        none    = 0,
        queue   = 1,  // Queue semantics (first in, first out)
        stack   = 2,  // Stack semantics (last in, first out)
        evict   = 4,  // Discard oldest item on overflow
        reject  = 8,  // Discard new item on overflow
    )

    template <std::movable T, RingPolicy P, std::size_t N = 0>
    class RingBuffer {

    public:

        using value_type = T;

        static constexpr RingPolicy policy = P;
        static constexpr std::size_t static_capacity = N;
        static constexpr bool is_static = N != 0;
        static constexpr bool is_queue = has_bit(P, RingPolicy::queue);
        static constexpr bool is_stack = has_bit(P, RingPolicy::stack);
        static constexpr bool is_evict = has_bit(P, RingPolicy::evict);
        static constexpr bool is_reject = has_bit(P, RingPolicy::reject);

        static_assert(is_queue != is_stack, "Ring buffer policy must be queue or stack");
        static_assert(is_evict != is_reject, "Ring buffer policy must be evict or reject");

        RingBuffer() noexcept requires is_static {}
        explicit RingBuffer(std::size_t n) requires (! is_static): spec_{n} {}
        ~RingBuffer() noexcept { clear(); }

        RingBuffer(const RingBuffer&) = delete;
        RingBuffer(RingBuffer&&) requires is_static = delete;
        RingBuffer(RingBuffer&&) requires (! is_static) = default;
        RingBuffer& operator=(const RingBuffer&) = delete;
        RingBuffer& operator=(RingBuffer&&) requires is_static = delete;
        RingBuffer& operator=(RingBuffer&&) requires (! is_static) = default;

        bool empty() const noexcept { return len_ == 0; }
        bool full() const noexcept { return len_ == spec_.cap; }
        std::size_t size() const noexcept { return len_; }
        std::size_t capacity() const noexcept { return spec_.cap; }
        void clear() noexcept;
        const T& next() const noexcept;
        void pop() noexcept;
        T pull() noexcept;
        bool push(const T& t);
        bool push(T&& t) noexcept;

    private:

        struct nonstatic_specific {
            std::size_t cap;
            std::vector<std::byte> mem;
            explicit nonstatic_specific(std::size_t n): cap{n}, mem{n * sizeof(T)} {}
        };

        struct static_specific {
            static constexpr std::size_t cap = N;
            alignas(T) std::array<std::byte, N * sizeof(T)> mem;
        };

        using specific_data = std::conditional_t<is_static, static_specific, nonstatic_specific>;

        specific_data spec_;
        std::size_t pos_ = 0;  // Offset of oldest entry
        std::size_t len_ = 0;  // Number of entries (may wrap around to beginning)

        T* data() noexcept { return reinterpret_cast<T*>(spec_.mem.data()); }
        const T* data() const noexcept { return reinterpret_cast<const T*>(spec_.mem.data()); }

    };

        template <std::movable T, RingPolicy P, std::size_t N>
        void RingBuffer<T, P, N>::clear() noexcept {
            if (pos_ + len_ <= capacity()) {
                for (auto index = pos_; index < pos_ + len_; ++index) {
                    data()[index].~T();
                }
            } else {
                for (auto index = pos_; index < capacity(); ++index) {
                    data()[index].~T();
                }
                for (auto index = 0uz; index < pos_ + len_ - capacity(); ++index) {
                    data()[index].~T();
                }
            }
            pos_ = len_ = 0;
        }

        template <std::movable T, RingPolicy P, std::size_t N>
        const T& RingBuffer<T, P, N>::next() const noexcept {
            if constexpr (is_queue) {
                return data()[pos_];
            } else {
                auto index = (pos_ + len_ - 1) % capacity();
                return data()[index];
            }
        }

        template <std::movable T, RingPolicy P, std::size_t N>
        void RingBuffer<T, P, N>::pop() noexcept {
            if constexpr (is_queue) {
                data()[pos_].~T();
                pos_ = (pos_ + 1) % capacity();
            } else {
                auto index = (pos_ + len_ - 1) % capacity();
                data()[index].~T();
            }
            --len_;
        }

        template <std::movable T, RingPolicy P, std::size_t N>
        T RingBuffer<T, P, N>::pull() noexcept {
            std::size_t index;
            if constexpr (is_queue) {
                index = pos_;
            } else {
                index = (pos_ + len_ - 1) % capacity();
            }
            auto t = std::move(data()[index]);
            data()[index].~T();
            if constexpr (is_queue) {
                pos_ = (pos_ + 1) % capacity();
            }
            --len_;
            return t;
        }

        template <std::movable T, RingPolicy P, std::size_t N>
        bool RingBuffer<T, P, N>::push(const T& t) {
            auto index = (pos_ + len_) % capacity();
            if (len_ < capacity()) {
                new (data() + index) T{t};
                ++len_;
                return true;
            }
            if constexpr (is_evict) {
                data()[pos_] = t;
                pos_ = (pos_ + 1) % capacity();
                return true;
            } else {
                return false;
            }
        }

        template <std::movable T, RingPolicy P, std::size_t N>
        bool RingBuffer<T, P, N>::push(T&& t) noexcept {
            auto index = (pos_ + len_) % capacity();
            if (len_ < capacity()) {
                new (data() + index) T{std::move(t)};
                ++len_;
                return true;
            }
            if constexpr (is_evict) {
                data()[pos_] = std::move(t);
                pos_ = (pos_ + 1) % capacity();
                return true;
            } else {
                return false;
            }
        }

}
