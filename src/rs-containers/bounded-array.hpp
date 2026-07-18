#pragma once

#include "rs-core/arithmetic.hpp"
#include "rs-core/iterator.hpp"
#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace RS::Containers {

    namespace Detail {

        template <typename CT>
        class BoundedArrayIterator:
        public Iterator<BoundedArrayIterator<CT>, CT, std::contiguous_iterator_tag> {

        public:

            BoundedArrayIterator() = default;
            explicit BoundedArrayIterator(CT* p) noexcept: ptr_(p) {}
            BoundedArrayIterator(const BoundedArrayIterator<std::remove_const_t<CT>>& i) noexcept:
                ptr_(i.ptr_) {}
            BoundedArrayIterator& operator=(const BoundedArrayIterator<std::remove_const_t<CT>>& i) noexcept
                { ptr_ = i.ptr_; return *this; }

            CT& operator*() const noexcept { return *ptr_; }
            BoundedArrayIterator& operator+=(std::ptrdiff_t rhs) noexcept { ptr_ += rhs; return *this; }
            std::ptrdiff_t operator-(BoundedArrayIterator rhs) const noexcept { return ptr_ - rhs.ptr_; }

        private:

            template <typename CT2> friend class BoundedArrayIterator;

            CT* ptr_ = nullptr;

        };

    }

    template <typename T, std::size_t N>
    class BoundedArray {

    public:

        using const_iterator = Detail::BoundedArrayIterator<const T>;
        using const_reference = const T&;
        using difference_type = std::ptrdiff_t;
        using iterator = Detail::BoundedArrayIterator<T>;
        using reference = T&;
        using size_type = std::size_t;
        using value_type = T;

        static constexpr std::size_t bound = N;

        BoundedArray() noexcept = default;
        explicit BoundedArray(std::size_t n, const T& t = {});
        template <std::input_iterator I> BoundedArray(I i, I j);
        BoundedArray(std::initializer_list<T> list);
        ~BoundedArray() noexcept { clear(); }
        BoundedArray(const BoundedArray& ba);
        BoundedArray(BoundedArray&& ba) noexcept;
        BoundedArray& operator=(const BoundedArray& ba);
        BoundedArray& operator=(BoundedArray&& ba) noexcept;
        BoundedArray& operator=(std::initializer_list<T> list);

        T& operator[](std::size_t i) noexcept { return data()[i]; }
        const T& operator[](std::size_t i) const noexcept { return data()[i]; }

        T& at(std::size_t i) { check_index(i); return data()[i]; }
        const T& at(std::size_t i) const { check_index(i); return data()[i]; }
        iterator begin() noexcept { return iterator(data()); }
        const_iterator begin() const noexcept { return cbegin(); }
        const_iterator cbegin() const noexcept { return const_iterator(cdata()); }
        iterator end() noexcept { return begin() + to_signed(num_); }
        const_iterator end() const noexcept { return cend(); }
        const_iterator cend() const noexcept { return cbegin() + to_signed(num_); }
        T* data() noexcept { return reinterpret_cast<T*>(mem_.data()); }
        const T* data() const noexcept { return cdata(); }
        const T* cdata() const noexcept { return reinterpret_cast<const T*>(mem_.data()); }
        T& front() noexcept { return *data(); }
        const T& front() const noexcept { return *cdata(); }
        T& back() noexcept { return data()[num_ - 1]; }
        const T& back() const noexcept { return cdata()[num_ - 1]; }
        std::size_t capacity() const noexcept { return N; }
        bool empty() const noexcept { return num_ == 0; }
        std::size_t size() const noexcept { return num_; }

        template <std::input_iterator I> iterator append(I i, I j);
        template <typename InputRange> iterator append(const InputRange& r);
        template <typename InputRange> iterator append(InputRange&& r);
        void clear() noexcept;
        template <typename... Args> iterator emplace(const_iterator i, Args&&... args);
        template <typename... Args> void emplace_back(Args&&... args);
        void erase(const_iterator i) noexcept;
        void erase(const_iterator i, const_iterator j) noexcept;
        iterator insert(const_iterator i, const T& t);
        iterator insert(const_iterator i, T&& t);
        template <std::input_iterator I> iterator insert(const_iterator i, I j, I k);
        void pop_back() noexcept;
        void push_back(const T& t);
        void push_back(T&& t);
        void resize(std::size_t n, const T& t = {});

        std::size_t hash() const noexcept;
        void swap(BoundedArray& ba) noexcept;

    private:

        alignas(T) std::array<std::byte, N * sizeof(T)> mem_;
        std::size_t num_ = 0;

        void check_index(std::size_t i) const;
        void check_length(std::size_t n) const;

    };

        template <typename T, std::size_t N>
        BoundedArray<T, N>::BoundedArray(std::size_t n, const T& t):
        num_{n} {
            check_length(n);
            std::uninitialized_fill(begin(), begin() + to_signed(n), t);
        }

        template <typename T, std::size_t N>
        template <std::input_iterator I>
        BoundedArray<T, N>::BoundedArray(I i, I j) {
            using namespace Detail;
            if constexpr (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i) {
                    push_back(*i);
                }
            } else {
                auto n = to_unsigned(std::distance(i, j));
                check_length(n);
                num_ = n;
                std::uninitialized_copy(i, j, begin());
            }
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>::BoundedArray(std::initializer_list<T> list):
        num_{list.size()} {
            check_length(num_);
            std::uninitialized_copy(list.begin(), list.end(), begin());
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>::BoundedArray(const BoundedArray& ba):
        num_{ba.num_} {
            std::uninitialized_copy(ba.begin(), ba.end(), begin());
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>::BoundedArray(BoundedArray&& ba) noexcept:
        num_{ba.num_} {
            using namespace Detail;
            std::uninitialized_move(ba.begin(), ba.end(), begin());
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>& BoundedArray<T, N>::operator=(const BoundedArray& ba) {
            BoundedArray temp{ba};
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>& BoundedArray<T, N>::operator=(BoundedArray&& ba) noexcept {
            BoundedArray temp{std::move(ba)};
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        BoundedArray<T, N>& BoundedArray<T, N>::operator=(std::initializer_list<T> list) {
            BoundedArray temp{list};
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        template <std::input_iterator I>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::append(I i, I j) {
            using namespace Detail;
            auto n_old = to_signed(num_);
            if (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i) {
                    push_back(*i);
                }
            } else {
                auto n_new = std::distance(i, j);
                check_length(num_ + to_unsigned(n_new));
                std::uninitialized_copy(i, j, begin() + n_old);
                num_ += to_unsigned(n_new);
            }
            return begin() + n_old;
        }

        template <typename T, std::size_t N>
        template <typename InputRange>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::append(const InputRange& r) {
            using std::begin;
            using std::end;
            return append(begin(r), end(r));
        }

        template <typename T, std::size_t N>
        template <typename InputRange>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::append(InputRange&& r) {
            using namespace Detail;
            using std::begin;
            using std::end;
            auto i = begin(r), j = end(r);
            auto n_old = to_signed(num_);
            if (std::is_same<typename std::iterator_traits<decltype(i)>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i) {
                    push_back(*i);
                }
            } else {
                auto n_new = std::distance(i, j);
                check_length(num_ + to_unsigned(n_new));
                std::uninitialized_move(i, j, this->begin() + n_old);
                num_ += to_unsigned(n_new);
            }
            return this->begin() + to_signed(n_old);
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::clear() noexcept {
            using namespace Detail;
            std::destroy(begin(), end());
            num_ = 0;
        }

        template <typename T, std::size_t N>
        template <typename... Args>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::emplace(const_iterator i, Args&&... args) {
            check_length(num_ + 1);
            auto pos = i - begin();
            if (pos < to_signed(num_)) {
                new (data() + num_) T(std::move(end()[-1]));
                std::move_backward(begin() + pos, end() - 1, end());
                begin()[pos].~T();
            }
            new (data() + pos) T(std::forward<Args>(args)...);
            ++num_;
            return begin() + pos;
        }

        template <typename T, std::size_t N>
        template <typename... Args>
        void BoundedArray<T, N>::emplace_back(Args&&... args) {
            check_length(num_ + 1);
            new (data() + to_signed(num_)) T(std::forward<Args>(args)...);
            ++num_;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::erase(const_iterator i) noexcept {
            auto x = i - cbegin();
            std::move(begin() + x + 1, end(), begin() + x);
            end()[-1].~T();
            --num_;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::erase(const_iterator i, const_iterator j) noexcept {
            using namespace Detail;
            auto n_erase = j - i;
            auto x = i - cbegin();
            auto y = j - cbegin();
            std::move(begin() + y, end(), begin() + x);
            std::destroy(end() - n_erase, end());
            num_ -= to_unsigned(n_erase);
        }

        template <typename T, std::size_t N>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::insert(const_iterator i, const T& t) {
            check_length(num_ + 1);
            auto pos = i - begin();
            if (pos < to_signed(num_)) {
                new (data() + num_) T(std::move(end()[-1]));
                std::move_backward(begin() + pos, end() - 1, end());
                begin()[pos] = t;
            } else {
                new (data() + pos) T(t);
            }
            ++num_;
            return begin() + pos;
        }

        template <typename T, std::size_t N>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::insert(const_iterator i, T&& t) {
            check_length(num_ + 1);
            auto pos = i - begin();
            if (pos < to_signed(num_)) {
                new (data() + num_) T(std::move(end()[-1]));
                std::move_backward(begin() + pos, end() - 1, end());
                begin()[pos] = std::move(t);
            } else {
                new (data() + pos) T(std::move(t));
            }
            ++num_;
            return begin() + pos;
        }

        template <typename T, std::size_t N>
        template <std::input_iterator I>
        typename BoundedArray<T, N>::iterator BoundedArray<T, N>::insert(const_iterator i, I j, I k) {
            using namespace Detail;
            auto n_before = i - begin();
            auto n_after = to_signed(num_) - n_before;
            if (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                BoundedArray temp(i, cend());
                erase(i, end());
                for (; j != k; ++j) {
                    push_back(*j);
                }
                append(std::move(temp));
            } else {
                auto n_inserted = std::distance(j, k);
                check_length(num_ + to_unsigned(n_inserted));
                auto insert_at = begin() + n_before;
                if (n_inserted < n_after) {
                    std::uninitialized_move(end() - n_inserted, end(), end());
                    std::move_backward(insert_at, end() - n_inserted, end());
                    std::copy(j, k, insert_at);
                } else {
                    std::uninitialized_move(insert_at, end(), end() + n_inserted - n_after);
                    auto mid = j;
                    std::advance(mid, n_after);
                    std::copy(j, mid, insert_at);
                    std::uninitialized_copy(mid, k, end());
                }
                num_ += to_unsigned(n_inserted);
            }
            return begin() + n_before;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::pop_back() noexcept {
            end()[-1].~T();
            --num_;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::push_back(const T& t) {
            check_length(num_ + 1);
            new (data() + num_) T(t);
            ++num_;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::push_back(T&& t) {
            check_length(num_ + 1);
            new (data() + num_) T(std::move(t));
            ++num_;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::resize(std::size_t n, const T& t) {
            check_length(n);
            if (n < num_) {
                std::destroy(begin() + to_signed(n), end());
            } else if (n > num_) {
                std::uninitialized_fill(begin() + to_signed(num_), begin() + to_signed(n), t);
            }
            num_ = n;
        }

        template <typename T, std::size_t N>
        std::size_t BoundedArray<T, N>::hash() const noexcept {
            std::size_t h = 0;
            std::hash<T> ht;
            for (const auto& t: *this) {
                h = (h << 1) + ht(t);
            }
            return h;
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::swap(BoundedArray& ba) noexcept {
            using namespace Detail;
            auto common = std::min(num_, ba.num_);
            auto signed_common = to_signed(common);
            std::swap_ranges(begin(), begin() + signed_common, ba.begin());
            if (num_ > common) {
                std::uninitialized_move(begin() + signed_common, end(), ba.begin() + signed_common);
                std::destroy(begin() + signed_common, end());
            } else if (ba.num_ > common) {
                std::uninitialized_move(ba.begin() + signed_common, ba.end(), begin() + signed_common);
                std::destroy(ba.begin() + signed_common, ba.end());
            }
            std::swap(num_, ba.num_);
        }

        template <typename T, std::size_t N>
        void swap(BoundedArray<T, N>& a, BoundedArray<T, N>& b) noexcept {
            a.swap(b);
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::check_index(std::size_t i) const {
            if (i >= num_) {
                throw std::out_of_range("Bounded array index out of bounds");
            }
        }

        template <typename T, std::size_t N>
        void BoundedArray<T, N>::check_length(std::size_t n) const {
            if (n > N) {
                throw std::length_error("Bounded array length exceeds bound");
            }
        }

        template <typename T, std::size_t N>
        bool operator==(const BoundedArray<T, N>& a, const BoundedArray<T, N>& b) noexcept {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
        }

        template <typename T, std::size_t N>
        auto operator<=>(const BoundedArray<T, N>& a, const BoundedArray<T, N>& b) noexcept {
            using namespace Detail;
            auto i = a.begin();
            auto j = a.end();
            auto k = b.begin();
            auto l = b.end();
            for (; i != j && k != l; ++i, ++j) {
                auto c = std::compare_three_way{}(*i, *k);
                if (c != 0) {
                    return c;
                }
            }
            if (*i != *j) {
                return std::strong_ordering::greater;
            } else if (*k != *l) {
                return std::strong_ordering::less;
            } else {
                return std::strong_ordering::equal;
            }
        }

}

template <typename T, std::size_t N>
struct std::hash<RS::Containers::BoundedArray<T, N>> {
    std::size_t operator()(const RS::Containers::BoundedArray<T, N>& v) const noexcept {
        return v.hash();
    }
};
