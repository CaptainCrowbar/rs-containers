#include "rs-containers/bounded-array.hpp"
#include "rs-core/unit-test.hpp"
#include <format>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace RS;
using namespace RS::Containers;

void test_rs_containers_bounded_array_construction() {

    using IBA = BoundedArray<int, 10>;

    IBA a;

    TEST(a.empty());
    TEST_EQUAL(a.size(), 0u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_THROW(a.at(0), std::out_of_range, "index out of bounds");
    TEST_EQUAL(std::format("{}", a), "[]");

    IBA b(5, 42);

    TEST(! b.empty());
    TEST_EQUAL(b.size(), 5u);
    TEST_EQUAL(b.capacity(), 10u);
    TEST_EQUAL(b[0], 42);
    TEST_EQUAL(b[4], 42);
    TEST_EQUAL(b.at(0), 42);
    TEST_EQUAL(b.at(4), 42);
    TEST_THROW(b.at(5), std::out_of_range, "index out of bounds");
    TEST_EQUAL(std::format("{}", b), "[42, 42, 42, 42, 42]");

    TRY(a = b);
    TEST(! a.empty());
    TEST(! b.empty());
    TEST_EQUAL(a.size(), 5u);
    TEST_EQUAL(b.size(), 5u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(b.capacity(), 10u);
    TEST_EQUAL(a[0], 42);
    TEST_EQUAL(a[4], 42);
    TEST_EQUAL(std::format("{}", a), "[42, 42, 42, 42, 42]");

    TRY(a = std::move(b));
    TEST(! a.empty());
    TEST(! b.empty());
    TEST_EQUAL(a.size(), 5u);
    TEST_EQUAL(b.size(), 5u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(b.capacity(), 10u);
    TEST_EQUAL(a[0], 42);
    TEST_EQUAL(a[4], 42);
    TEST_EQUAL(std::format("{}", a), "[42, 42, 42, 42, 42]");

    IBA c(10, 42);

    TEST(! c.empty());
    TEST_EQUAL(c.size(), 10u);
    TEST_EQUAL(c.capacity(), 10u);
    TEST_EQUAL(c[0], 42);
    TEST_EQUAL(c[9], 42);
    TEST_EQUAL(std::format("{}", c), "[42, 42, 42, 42, 42, 42, 42, 42, 42, 42]");

    TRY(a = c);
    TEST(! a.empty());
    TEST(! c.empty());
    TEST_EQUAL(a.size(), 10u);
    TEST_EQUAL(c.size(), 10u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(c.capacity(), 10u);
    TEST_EQUAL(a[0], 42);
    TEST_EQUAL(a[4], 42);
    TEST_EQUAL(std::format("{}", a), "[42, 42, 42, 42, 42, 42, 42, 42, 42, 42]");

    TRY(a = std::move(c));
    TEST(! a.empty());
    TEST(! c.empty());
    TEST_EQUAL(a.size(), 10u);
    TEST_EQUAL(c.size(), 10u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(c.capacity(), 10u);
    TEST_EQUAL(a[0], 42);
    TEST_EQUAL(a[4], 42);
    TEST_EQUAL(std::format("{}", a), "[42, 42, 42, 42, 42, 42, 42, 42, 42, 42]");

    TRY((a = IBA{1, 2, 3, 4, 5}));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 5u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[4], 5);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5]");
    TEST_EQUAL(a.front(), 1);
    TEST_EQUAL(a.back(), 5);

    TRY((a = IBA{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 10u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[9], 10);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]");

    TRY((a = {1, 2, 3, 4, 5}));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 5u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[4], 5);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5]");

    TRY((a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 10u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[9], 10);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]");

    std::vector<int> v = {1, 2, 3, 4, 5};
    TRY(a = IBA(v.begin(), v.end()));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 5u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[4], 5);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5]");

    v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    TRY(a = IBA(v.begin(), v.end()));
    TEST(! a.empty());
    TEST_EQUAL(a.size(), 10u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(a[0], 1);
    TEST_EQUAL(a[9], 10);
    TEST_EQUAL(std::format("{}", a), "[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]");

    TRY(a.clear());
    TEST(a.empty());
    TEST_EQUAL(a.size(), 0u);
    TEST_EQUAL(a.capacity(), 10u);
    TEST_EQUAL(std::format("{}", a), "[]");

}
