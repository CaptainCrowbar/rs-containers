#pragma once

#include "rs-core/global.hpp"
#include "rs-core/iterator.hpp"
#include <algorithm>
#include <bit>
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
        class CompactArrayIterator:
        public Iterator<CompactArrayIterator<CT>, CT, std::contiguous_iterator_tag> {

        public:

            CompactArrayIterator() = default;
            explicit CompactArrayIterator(CT* p) noexcept: ptr_(p) {}
            CompactArrayIterator(const CompactArrayIterator<std::remove_const_t<CT>>& i) noexcept:
                ptr_(i.ptr_) {}
            CompactArrayIterator& operator=(const CompactArrayIterator<std::remove_const_t<CT>>& i) noexcept
                { ptr_ = i.ptr_; return *this; }

            CT& operator*() const noexcept { return *ptr_; }
            CompactArrayIterator& operator+=(std::ptrdiff_t rhs) noexcept { ptr_ += rhs; return *this; }
            std::ptrdiff_t operator-(CompactArrayIterator rhs) const noexcept { return ptr_ - rhs.ptr_; }

        private:

            template <typename CT2> friend class CompactArrayIterator;

            CT* ptr_ = nullptr;

        };

    }

    template <typename T, std::size_t N>
    class CompactArray {

    public:

        using const_iterator = Detail::CompactArrayIterator<const T>;
        using const_reference = const T&;
        using difference_type = std::ptrdiff_t;
        using iterator = Detail::CompactArrayIterator<T>;
        using reference = T&;
        using size_type = std::size_t;
        using value_type = T;

        static constexpr std::size_t threshold = N;

        CompactArray() noexcept = default;
        explicit CompactArray(std::size_t n, const T& t = {});
        template <std::input_iterator I> CompactArray(I i, I j);
        CompactArray(std::initializer_list<T> list);
        ~CompactArray() noexcept { clear(); }
        CompactArray(const CompactArray& ca);
        CompactArray(CompactArray&& ca) noexcept;
        CompactArray& operator=(const CompactArray& ca);
        CompactArray& operator=(CompactArray&& ca) noexcept;
        CompactArray& operator=(std::initializer_list<T> list);

        T& operator[](std::size_t i) noexcept { return data()[i]; }
        const T& operator[](std::size_t i) const noexcept { return data()[i]; }

        T& at(std::size_t i) { check_index(i); return data()[i]; }
        const T& at(std::size_t i) const { check_index(i); return data()[i]; }
        iterator begin() noexcept { return iterator(data()); }
        const_iterator begin() const noexcept { return const_iterator(cdata()); }
        const_iterator cbegin() const noexcept { return const_iterator(cdata()); }
        iterator end() noexcept { return begin() + static_cast<std::ptrdiff_t>(num_); }
        const_iterator end() const noexcept { return begin() + static_cast<std::ptrdiff_t>(num_); }
        const_iterator cend() const noexcept { return cbegin() + static_cast<std::ptrdiff_t>(num_); }
        T* data() noexcept { return reinterpret_cast<T*>(local_ ? uni_.mem : uni_.pc.ptr); }
        const T* data() const noexcept { return cdata(); }
        const T* cdata() const noexcept { return reinterpret_cast<const T*>(local_ ? uni_.mem : uni_.pc.ptr); }
        T& front() noexcept { return *data(); }
        const T& front() const noexcept { return *cdata(); }
        T& back() noexcept { return data()[num_ - 1]; }
        const T& back() const noexcept { return cdata()[num_ - 1]; }
        std::size_t capacity() const noexcept { return local_ ? N : uni_.pc.cap; }
        bool empty() const noexcept { return num_ == 0; }
        bool is_compact() const noexcept { return local_; }
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
        void reserve(std::size_t n);
        void resize(std::size_t n, const T& t = {});
        void shrink_to_fit();

        std::size_t hash() const noexcept;
        void swap(CompactArray& ca) noexcept;

    private:

        using raw_memory = struct {
            alignas(T) std::byte bytes[sizeof(T)];
        };

        union {
            raw_memory mem[N];
            struct {
                raw_memory* ptr;
                std::size_t cap;
            } pc;
        } uni_;

        std::size_t num_ = 0;
        bool local_ = true;

        void check_index(std::size_t i) const;

    };

        template <typename T, std::size_t N>
        CompactArray<T, N>::CompactArray(std::size_t n, const T& t) {
            reserve(n);
            std::uninitialized_fill(begin(), begin() + static_cast<std::ptrdiff_t>(n), t);
            num_ = n;
        }

        template <typename T, std::size_t N>
        template <std::input_iterator I>
        CompactArray<T, N>::CompactArray(I i, I j) {
            using namespace Detail;
            if (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i)
                    push_back(*i);
            } else {
                auto n = static_cast<std::size_t>(std::distance(i, j));
                reserve(n);
                std::uninitialized_copy(i, j, begin());
                num_ = n;
            }
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>::CompactArray(std::initializer_list<T> list) {
            reserve(list.size());
            std::uninitialized_copy(list.begin(), list.end(), begin());
            num_ = list.size();
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>::CompactArray(const CompactArray& ca) {
            reserve(ca.size());
            std::uninitialized_copy(ca.begin(), ca.end(), begin());
            num_ = ca.num_;
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>::CompactArray(CompactArray&& ca) noexcept {
            using namespace Detail;
            reserve(ca.size());
            std::uninitialized_move(ca.begin(), ca.end(), begin());
            num_ = ca.num_;
            ca.clear();
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>& CompactArray<T, N>::operator=(const CompactArray& ca) {
            CompactArray temp(ca);
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>& CompactArray<T, N>::operator=(CompactArray&& ca) noexcept {
            CompactArray temp(std::move(ca));
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        CompactArray<T, N>& CompactArray<T, N>::operator=(std::initializer_list<T> list) {
            CompactArray temp(list);
            swap(temp);
            return *this;
        }

        template <typename T, std::size_t N>
        template <std::input_iterator I>
        typename CompactArray<T, N>::iterator CompactArray<T, N>::append(I i, I j) {
            using namespace Detail;
            auto n_old = static_cast<std::ptrdiff_t>(num_);
            if (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i)
                    push_back(*i);
            } else {
                auto n_new = static_cast<std::size_t>(std::distance(i, j));
                reserve(num_ + n_new);
                std::uninitialized_copy(i, j, begin() + n_old);
                num_ += n_new;
            }
            return begin() + n_old;
        }

        template <typename T, std::size_t N>
        template <typename InputRange>
        typename CompactArray<T, N>::iterator CompactArray<T, N>::append(const InputRange& r) {
            using std::begin;
            using std::end;
            return append(begin(r), end(r));
        }

        template <typename T, std::size_t N>
        template <typename InputRange>
        typename CompactArray<T, N>::iterator CompactArray<T, N>::append(InputRange&& r) {
            using namespace Detail;
            using std::begin;
            using std::end;
            auto i = begin(r), j = end(r);
            auto n_old = static_cast<std::ptrdiff_t>(num_);
            if (std::is_same<typename std::iterator_traits<decltype(i)>::iterator_category, std::input_iterator_tag>::value) {
                for (; i != j; ++i)
                    push_back(*i);
            } else {
                auto n_new = static_cast<std::size_t>(std::distance(i, j));
                reserve(num_ + n_new);
                std::uninitialized_move(i, j, this->begin() + n_old);
                num_ += n_new;
            }
            return this->begin() + n_old;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::clear() noexcept {
            using namespace Detail;
            std::destroy(begin(), end());
            if (! local_)
                delete[] uni_.pc.ptr;
            num_ = 0;
            local_ = true;
        }

        template <typename T, std::size_t N>
        template <typename... Args>
        typename CompactArray<T, N>::iterator CompactArray<T, N>::emplace(const_iterator i, Args&&... args) {
            auto pos = i - begin();
            reserve(num_ + 1);
            if (static_cast<std::size_t>(pos) < num_) {
                new (data() + num_) T(std::move(end()[-1]));
                std::move_backward(begin() + pos, end() - 1, end());
                begin()[pos].~T();
            }
            new (data() + pos) T(args...);
            ++num_;
            return begin() + pos;
        }

        template <typename T, std::size_t N>
        template <typename... Args>
        void CompactArray<T, N>::emplace_back(Args&&... args) {
            reserve(num_ + 1);
            new (data() + num_) T(args...);
            ++num_;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::erase(const_iterator i) noexcept {
            auto x = i - cbegin();
            std::move(begin() + x + 1, end(), begin() + x);
            end()[-1].~T();
            --num_;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::erase(const_iterator i, const_iterator j) noexcept {
            using namespace Detail;
            auto n_erase = j - i;
            auto x = i - cbegin();
            auto y = j - cbegin();
            std::move(begin() + y, end(), begin() + x);
            std::destroy(end() - n_erase, end());
            num_ -= static_cast<std::size_t>(n_erase);
        }

        template <typename T, std::size_t N>
        typename CompactArray<T, N>::iterator CompactArray<T, N>::insert(const_iterator i, const T& t) {
            auto pos = i - begin();
            reserve(num_ + 1);
            if (pos < static_cast<std::ptrdiff_t>(num_)) {
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
        typename CompactArray<T, N>::iterator CompactArray<T, N>::insert(const_iterator i, T&& t) {
            auto pos = i - begin();
            reserve(num_ + 1);
            if (static_cast<std::size_t>(pos) < num_) {
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
        typename CompactArray<T, N>::iterator CompactArray<T, N>::insert(const_iterator i, I j, I k) {
            using namespace Detail;
            auto n_before = i - begin();
            auto n_after = static_cast<std::ptrdiff_t>(num_) - n_before;
            if (std::is_same<typename std::iterator_traits<I>::iterator_category, std::input_iterator_tag>::value) {
                CompactArray temp(i, cend());
                erase(i, end());
                for (; j != k; ++j)
                    push_back(*j);
                append(std::move(temp));
            } else {
                auto n_inserted = std::distance(j, k);
                reserve(num_ + static_cast<std::size_t>(n_inserted));
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
                num_ += static_cast<std::size_t>(n_inserted);
            }
            return begin() + n_before;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::pop_back() noexcept {
            end()[-1].~T();
            --num_;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::push_back(const T& t) {
            reserve(num_ + 1);
            new (data() + num_) T(t);
            ++num_;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::push_back(T&& t) {
            reserve(num_ + 1);
            new (data() + num_) T(std::move(t));
            ++num_;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::reserve(std::size_t n) {
            using namespace Detail;
            if (n <= num_) {
                std::destroy(begin() + static_cast<std::ptrdiff_t>(n), end());
                num_ = n;
                return;
            }
            if (n <= N)
                return;
            std::size_t new_cap = std::bit_ceil(n);
            if (new_cap <= capacity())
                return;
            auto new_ptr = new raw_memory[new_cap];
            auto new_t_ptr = reinterpret_cast<T*>(new_ptr);
            std::uninitialized_move(begin(), end(), new_t_ptr);
            std::destroy(begin(), end());
            if (! local_)
                delete[] uni_.pc.ptr;
            uni_.pc.ptr = new_ptr;
            uni_.pc.cap = new_cap;
            local_ = false;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::resize(std::size_t n, const T& t) {
            reserve(n);
            if (n > num_) {
                std::uninitialized_fill(begin() + static_cast<std::ptrdiff_t>(num_),
                    begin() + static_cast<std::ptrdiff_t>(n), t);
                num_ = n;
            }
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::shrink_to_fit() {
            using namespace Detail;
            if (local_) {
                // Already at minimum capacity, nothing to do
            } else if (num_ <= N) {
                auto old_ptr = uni_.pc.ptr;
                auto old_t_ptr = reinterpret_cast<T*>(old_ptr);
                local_ = true;
                std::uninitialized_move(old_t_ptr, old_t_ptr + num_, begin());
                std::destroy(old_t_ptr, old_t_ptr + num_);
                delete[] old_ptr;
            } else {
                auto new_ptr = new raw_memory[num_];
                auto new_t_ptr = reinterpret_cast<T*>(new_ptr);
                std::uninitialized_move(begin(), end(), new_t_ptr);
                std::destroy(begin(), end());
                delete[] uni_.pc.ptr;
                uni_.pc.ptr = new_ptr;
                uni_.pc.cap = num_;
            }
        }

        template <typename T, std::size_t N>
        std::size_t CompactArray<T, N>::hash() const noexcept {
            std::size_t h = 0;
            std::hash<T> ht;
            for (auto& t: *this)
                h = (h << 1) + ht(t);
            return h;
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::swap(CompactArray& ca) noexcept {
            using namespace Detail;
            if (local_ && ca.local_) {
                auto common = std::min(num_, ca.num_);
                auto signed_common = static_cast<std::ptrdiff_t>(common);
                std::swap_ranges(begin(), begin() + signed_common, ca.begin());
                if (num_ > common) {
                    std::uninitialized_move(begin() + signed_common, end(), ca.begin() + signed_common);
                    std::destroy(begin() + signed_common, end());
                } else if (ca.num_ > common) {
                    std::uninitialized_move(ca.begin() + signed_common, ca.end(), begin() + signed_common);
                    std::destroy(ca.begin() + signed_common, ca.end());
                }
                std::swap(num_, ca.num_);
            } else if (local_) {
                auto p = ca.uni_.pc.ptr;
                auto c = ca.uni_.pc.cap;
                auto ca_ptr = reinterpret_cast<T*>(ca.uni_.mem);
                std::uninitialized_move(begin(), end(), ca_ptr);
                std::destroy(begin(), end());
                std::swap(num_, ca.num_);
                std::swap(local_, ca.local_);
                uni_.pc.ptr = p;
                uni_.pc.cap = c;
            } else if (ca.local_) {
                ca.swap(*this);
            } else {
                std::swap(uni_.pc.ptr, ca.uni_.pc.ptr);
                std::swap(uni_.pc.cap, ca.uni_.pc.cap);
                std::swap(num_, ca.num_);
            }
        }

        template <typename T, std::size_t N>
        void swap(CompactArray<T, N>& a, CompactArray<T, N>& b) noexcept {
            a.swap(b);
        }

        template <typename T, std::size_t N>
        void CompactArray<T, N>::check_index(std::size_t i) const {
            if (i >= num_) {
                throw std::out_of_range("Compact array index out of bounds");
            }
        }

        template <typename T, std::size_t N>
        bool operator==(const CompactArray<T, N>& a, const CompactArray<T, N>& b) noexcept {
            return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
        }

        template <typename T, std::size_t N>
        auto operator<=>(const CompactArray<T, N>& a, const CompactArray<T, N>& b) noexcept {
            using namespace Detail;
            auto i = a.begin();
            auto j = a.end();
            auto k = b.begin();
            auto l = b.end();
            for (; i != j && k != l; ++i, ++j) {
                auto c = std::compare_three_way{}(*i, *k);
                if (c != 0)
                    return c;
            }
            if (*i != *j)
                return std::strong_ordering::greater;
            else if (*k != *l)
                return std::strong_ordering::less;
            else
                return std::strong_ordering::equal;
        }

}

template <typename T, std::size_t N>
struct std::hash<RS::Containers::CompactArray<T, N>> {
    std::size_t operator()(const RS::Containers::CompactArray<T, N>& v) const noexcept {
        return v.hash();
    }
};
