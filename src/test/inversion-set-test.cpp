#include "rs-containers/inversion-list.hpp"
#include "rs-core/unit-test.hpp"
#include <format>

using namespace RS;
using namespace RS::Containers;

void test_rs_containers_inversion_set() {

    InversionSet<int> set;

    TEST(set.empty());
    TEST_EQUAL(set.size(), 0u);
    TEST_EQUAL(std::format("{}", set), "{}");
    TEST(! set.contains(1));

    TRY((set = {{10, 20}}));
    TEST(! set.empty());
    TEST_EQUAL(set.size(), 1u);
    TEST_EQUAL(std::format("{}", set), "{10:20}");
    TEST(! set.contains(0));
    TEST(! set.contains(9));
    TEST(set.contains(10));
    TEST(set.contains(15));
    TEST(set.contains(20));
    TEST(! set.contains(21));
    TEST(! set.contains(30));

    TRY((set = {{10, 20}, {30, 40}, {50, 60}}));
    TEST_EQUAL(set.size(), 3u);
    TEST_EQUAL(std::format("{}", set), "{10:20,30:40,50:60}");
    TEST(! set.empty());
    TEST(! set.contains(9));
    TEST(set.contains(10));
    TEST(set.contains(20));
    TEST(! set.contains(21));
    TEST(! set.contains(29));
    TEST(set.contains(30));
    TEST(set.contains(40));
    TEST(! set.contains(41));
    TEST(! set.contains(49));
    TEST(set.contains(50));
    TEST(set.contains(60));
    TEST(! set.contains(61));

    TRY((set = {{50, 60}, {30, 40}, {10, 20}}));
    TEST_EQUAL(set.size(), 3u);
    TEST_EQUAL(std::format("{}", set), "{10:20,30:40,50:60}");

    TRY((set = {{10, 20}, {30, 40}, {50, 60}, {50, 60}, {30, 40}, {10, 20}}));
    TEST_EQUAL(set.size(), 6u);
    TEST_EQUAL(std::format("{}", set), "{10:20,10:20,30:40,30:40,50:60,50:60}");

    TRY((set = {{60, 50}, {40, 30}, {20, 10}}));
    TEST(set.empty());

}
