#include "rs-containers/ring-buffer.hpp"
#include "rs-core/arithmetic.hpp"
#include "rs-core/unit-test.hpp"
#include <algorithm>
#include <cstddef>
#include <string>

using namespace RS;
using namespace RS::Containers;
using CI = RS::UnitTest::Counted<int>;

void test_rs_containers_ring_buffer_basic_queue_eject() {

    CI::reset();

    {

        RingBuffer<CI, RingPolicy::queue | RingPolicy::evict> buf {5};
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(CI::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 1);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 2);  TEST(buf.full());

        TEST_EQUAL(buf.next().get(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 6);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().get(), 2);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 3);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 4);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 5);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 6);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));

        for (auto i = 4; i <= 20; ++i) {
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(CI::count(), 3);
            TRY(buf.push(i));
            TEST_EQUAL(buf.size(), 4u);
            TEST_EQUAL(CI::count(), 4);
            TEST_EQUAL(buf.next().get(), i - 3);
            TRY(buf.pop());
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(CI::count(), 3);
            TEST_EQUAL(buf.next().get(), i - 2);
        }

        TRY(buf.clear());
        TEST(buf.empty());
        TEST_EQUAL(CI::count(), 0);

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), to_unsigned(std::min(i, 5)));
            TEST_EQUAL(CI::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(CI::count(), 0);
        }

    }

    TEST_EQUAL(CI::count(), 0);

}

void test_rs_containers_ring_buffer_basic_queue_reject() {

    CI::reset();

    {

        RingBuffer<CI, RingPolicy::queue | RingPolicy::reject> buf {5};
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(CI::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 1);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 1);  TEST(buf.full());

        TEST_EQUAL(buf.next().get(), 1);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().get(), 1);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 2);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 3);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 4);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 5);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));

        for (auto i = 4; i <= 20; ++i) {
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(CI::count(), 3);
            TRY(buf.push(i));
            TEST_EQUAL(buf.size(), 4u);
            TEST_EQUAL(CI::count(), 4);
            TEST_EQUAL(buf.next().get(), i - 3);
            TRY(buf.pop());
            TEST_EQUAL(buf.size(), 3u);
            TEST_EQUAL(CI::count(), 3);
            TEST_EQUAL(buf.next().get(), i - 2);
        }

        TRY(buf.clear());
        TEST(buf.empty());
        TEST_EQUAL(CI::count(), 0);

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), to_unsigned(std::min(i, 5)));
            TEST_EQUAL(CI::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(CI::count(), 0);
        }

    }

    TEST_EQUAL(CI::count(), 0);

}

void test_rs_containers_ring_buffer_basic_stack_eject() {

    CI::reset();

    {

        RingBuffer<CI, RingPolicy::stack | RingPolicy::evict> buf {5};
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(CI::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST_EQUAL(buf.next().get(), 2);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST_EQUAL(buf.next().get(), 3);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST_EQUAL(buf.next().get(), 4);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 5);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 6);  TEST(buf.full());

        TEST_EQUAL(buf.next().get(), 6);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().get(), 6);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 5);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 4);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 3);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 2);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), to_unsigned(std::min(i, 5)));
            TEST_EQUAL(CI::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(CI::count(), 0);
        }

    }

    TEST_EQUAL(CI::count(), 0);

}

void test_rs_containers_ring_buffer_basic_stack_reject() {

    CI::reset();

    {

        RingBuffer<CI, RingPolicy::stack | RingPolicy::reject> buf {5};
        std::string str;

        TEST(buf.empty());
        TEST_EQUAL(buf.size(), 0u);
        TEST_EQUAL(buf.capacity(), 5u);
        TEST_EQUAL(CI::count(), 0);

        TRY(buf.push(1));  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST_EQUAL(buf.next().get(), 1);  TEST(! buf.full());
        TRY(buf.push(2));  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST_EQUAL(buf.next().get(), 2);  TEST(! buf.full());
        TRY(buf.push(3));  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST_EQUAL(buf.next().get(), 3);  TEST(! buf.full());
        TRY(buf.push(4));  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST_EQUAL(buf.next().get(), 4);  TEST(! buf.full());
        TRY(buf.push(5));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 5);  TEST(buf.full());
        TRY(buf.push(6));  TEST_EQUAL(buf.size(), 5u);  TEST_EQUAL(CI::count(), 5);  TEST_EQUAL(buf.next().get(), 5);  TEST(buf.full());

        TEST_EQUAL(buf.next().get(), 5);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 4);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 3);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 2);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.next().get(), 1);  TRY(buf.pop());  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        TRY(buf.push(1));
        TRY(buf.push(2));
        TRY(buf.push(3));
        TRY(buf.push(4));
        TRY(buf.push(5));
        TRY(buf.push(6));

        TEST_EQUAL(buf.pull().get(), 5);  TEST_EQUAL(buf.size(), 4u);  TEST_EQUAL(CI::count(), 4);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 4);  TEST_EQUAL(buf.size(), 3u);  TEST_EQUAL(CI::count(), 3);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 3);  TEST_EQUAL(buf.size(), 2u);  TEST_EQUAL(CI::count(), 2);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 2);  TEST_EQUAL(buf.size(), 1u);  TEST_EQUAL(CI::count(), 1);  TEST(! buf.empty());
        TEST_EQUAL(buf.pull().get(), 1);  TEST_EQUAL(buf.size(), 0u);  TEST_EQUAL(CI::count(), 0);  TEST(buf.empty());

        for (auto i = 0; i <= 10; ++i) {
            for (auto j = 1; j <= i; ++j) {
                TRY(buf.push(j));
            }
            TEST_EQUAL(buf.size(), to_unsigned(std::min(i, 5)));
            TEST_EQUAL(CI::count(), std::min(i, 5));
            TRY(buf.clear());
            TEST(buf.empty());
            TEST_EQUAL(CI::count(), 0);
        }

    }

    TEST_EQUAL(CI::count(), 0);

}
