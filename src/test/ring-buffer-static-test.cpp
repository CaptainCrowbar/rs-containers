#include "rs-containers/ring-buffer.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

using namespace RS::Containers;

namespace {

    class Tracked {
    public:
        Tracked() { ++count_; }
        Tracked(int n): num_{n} { ++count_; }
        ~Tracked() { --count_; }
        Tracked(const Tracked& t): num_{t.num_} { ++count_; }
        Tracked(Tracked&& t): num_{std::exchange(t.num_, 0)} { ++count_; }
        Tracked& operator=(const Tracked& t) { num_ = t.num_; return *this; }
        Tracked& operator=(Tracked&& t) { num_ = std::exchange(t.num_, 0); return *this; }
        int num() const { return num_; }
        static int count() { return count_; }
        static void reset() { count_ = 0; }
    private:
        int num_ = 0;
        static int count_;
    };

    int Tracked::count_ = 0;

}

void test_rs_containers_ring_buffer_static_queue_eject() {

    Tracked::reset();

    {

        RingBuffer<Tracked, RingPolicy::queue | RingPolicy::evict, 5> buf;
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(Tracked::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 1);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 2);  TEST(buf.full());

        TEST_EQUAL(buf.next().num(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 6);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().num(), 2);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 3);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 4);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 5);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 6);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));

        for (auto i = 4; i <= 20; ++i) {
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(Tracked::count(), 3);
            TRY(buf.push(i));
            TEST_EQUAL(buf.size(), 4u);
            TEST_EQUAL(Tracked::count(), 4);
            TEST_EQUAL(buf.next().num(), i - 3);
            TRY(buf.pop());
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(Tracked::count(), 3);
            TEST_EQUAL(buf.next().num(), i - 2);
        }

        TRY(buf.clear());
        TEST(buf.empty());
        TEST_EQUAL(Tracked::count(), 0);

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), static_cast<std::size_t>(std::min(i, 5)));
            TEST_EQUAL(Tracked::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(Tracked::count(), 0);
        }

    }

    TEST_EQUAL(Tracked::count(), 0);

}

void test_rs_containers_ring_buffer_static_queue_reject() {

    Tracked::reset();

    {

        RingBuffer<Tracked, RingPolicy::queue | RingPolicy::reject, 5> buf;
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(Tracked::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 1);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 1);  TEST(buf.full());

        TEST_EQUAL(buf.next().num(), 1);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().num(), 1);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 2);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 3);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 4);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 5);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));

        for (auto i = 4; i <= 20; ++i) {
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(Tracked::count(), 3);
            TRY(buf.push(i));
            TEST_EQUAL(buf.size(), 4u);
            TEST_EQUAL(Tracked::count(), 4);
            TEST_EQUAL(buf.next().num(), i - 3);
            TRY(buf.pop());
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(Tracked::count(), 3);
            TEST_EQUAL(buf.next().num(), i - 2);
        }

        TRY(buf.clear());
        TEST(buf.empty());
        TEST_EQUAL(Tracked::count(), 0);

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), static_cast<std::size_t>(std::min(i, 5)));
            TEST_EQUAL(Tracked::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(Tracked::count(), 0);
        }

    }

    TEST_EQUAL(Tracked::count(), 0);

}

void test_rs_containers_ring_buffer_static_stack_eject() {

    Tracked::reset();

    {

        RingBuffer<Tracked, RingPolicy::stack | RingPolicy::evict, 5> buf;
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(Tracked::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST_EQUAL(buf.next().num(), 2);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST_EQUAL(buf.next().num(), 3);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST_EQUAL(buf.next().num(), 4);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 5);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 6);  TEST(buf.full());

        TEST_EQUAL(buf.next().num(), 6);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().num(), 6);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 5);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 4);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 3);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 2);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), static_cast<std::size_t>(std::min(i, 5)));
            TEST_EQUAL(Tracked::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(Tracked::count(), 0);
        }

    }

    TEST_EQUAL(Tracked::count(), 0);

}

void test_rs_containers_ring_buffer_static_stack_reject() {

    Tracked::reset();

    {

        RingBuffer<Tracked, RingPolicy::stack | RingPolicy::reject, 5> buf;
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(Tracked::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST_EQUAL(buf.next().num(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST_EQUAL(buf.next().num(), 2);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST_EQUAL(buf.next().num(), 3);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST_EQUAL(buf.next().num(), 4);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 5);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(Tracked::count(), 5);  TEST_EQUAL(buf.next().num(), 5);  TEST(buf.full());

        TEST_EQUAL(buf.next().num(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().num(), 1);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().num(), 5);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(Tracked::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 4);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(Tracked::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 3);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(Tracked::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 2);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(Tracked::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().num(), 1);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(Tracked::count(), 0);  TEST(buf.empty());

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), static_cast<std::size_t>(std::min(i, 5)));
            TEST_EQUAL(Tracked::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(Tracked::count(), 0);
        }

    }

    TEST_EQUAL(Tracked::count(), 0);

}
