#include "rs-containers/inversion-list.hpp"
#include "rs-core/unit-test.hpp"
#include <format>
#include <string>

using namespace RS;
using namespace RS::Containers;

void test_rs_containers_inversion_map() {

    using Imap = InversionMap<int, std::string>;

    Imap map;

    TEST(map.empty());
    TEST_EQUAL(map.size(), 0u);
    TEST_EQUAL(map.default_value(), "");
    TEST_EQUAL(std::format("{}", map), R"({default:""})");
    TEST(! map.contains(1));
    TEST_EQUAL(map[1], "");

    TRY((map = {{10, 20, "abc"}}));
    TEST(! map.empty());
    TEST_EQUAL(map.size(), 1u);
    TEST_EQUAL(map.default_value(), "");
    TEST_EQUAL(std::format("{}", map), R"({10:20:"abc",default:""})");

    TEST(! map.contains(0));   TEST_EQUAL(map[0], "");
    TEST(! map.contains(9));   TEST_EQUAL(map[9], "");
    TEST(map.contains(10));    TEST_EQUAL(map[10], "abc");
    TEST(map.contains(15));    TEST_EQUAL(map[15], "abc");
    TEST(map.contains(20));    TEST_EQUAL(map[20], "abc");
    TEST(! map.contains(21));  TEST_EQUAL(map[21], "");
    TEST(! map.contains(30));  TEST_EQUAL(map[30], "");

    TRY((map = {{10, 20, "abc"}, {30, 40, "def"}, {50, 60, "ghi"}}));
    TEST_EQUAL(map.size(), 3u);
    TEST_EQUAL(map.default_value(), "");
    TEST_EQUAL(std::format("{}", map), R"({10:20:"abc",30:40:"def",50:60:"ghi",default:""})");

    TEST(! map.contains(9));   TEST_EQUAL(map[9], "");
    TEST(map.contains(10));    TEST_EQUAL(map[10], "abc");
    TEST(map.contains(20));    TEST_EQUAL(map[20], "abc");
    TEST(! map.contains(21));  TEST_EQUAL(map[21], "");
    TEST(! map.contains(29));  TEST_EQUAL(map[29], "");
    TEST(map.contains(30));    TEST_EQUAL(map[30], "def");
    TEST(map.contains(40));    TEST_EQUAL(map[40], "def");
    TEST(! map.contains(41));  TEST_EQUAL(map[41], "");
    TEST(! map.contains(49));  TEST_EQUAL(map[49], "");
    TEST(map.contains(50));    TEST_EQUAL(map[50], "ghi");
    TEST(map.contains(60));    TEST_EQUAL(map[60], "ghi");
    TEST(! map.contains(61));  TEST_EQUAL(map[61], "");

    TRY((map = Imap{{{10, 20, "abc"}, {30, 40, "def"}, {50, 60, "ghi"}}, "xyz"}));
    TEST_EQUAL(map.size(), 3u);
    TEST_EQUAL(map.default_value(), "xyz");
    TEST_EQUAL(std::format("{}", map), R"({10:20:"abc",30:40:"def",50:60:"ghi",default:"xyz"})");

    TEST(! map.contains(9));   TEST_EQUAL(map[9], "xyz");
    TEST(map.contains(10));    TEST_EQUAL(map[10], "abc");
    TEST(map.contains(20));    TEST_EQUAL(map[20], "abc");
    TEST(! map.contains(21));  TEST_EQUAL(map[21], "xyz");
    TEST(! map.contains(29));  TEST_EQUAL(map[29], "xyz");
    TEST(map.contains(30));    TEST_EQUAL(map[30], "def");
    TEST(map.contains(40));    TEST_EQUAL(map[40], "def");
    TEST(! map.contains(41));  TEST_EQUAL(map[41], "xyz");
    TEST(! map.contains(49));  TEST_EQUAL(map[49], "xyz");
    TEST(map.contains(50));    TEST_EQUAL(map[50], "ghi");
    TEST(map.contains(60));    TEST_EQUAL(map[60], "ghi");
    TEST(! map.contains(61));  TEST_EQUAL(map[61], "xyz");

    TRY((map = Imap{{{50, 60, "ghi"}, {30, 40, "def"}, {10, 20, "abc"}}, "xyz"}));
    TEST_EQUAL(map.size(), 3u);
    TEST_EQUAL(map.default_value(), "xyz");
    TEST_EQUAL(std::format("{}", map), R"({10:20:"abc",30:40:"def",50:60:"ghi",default:"xyz"})");

    TRY((map = Imap{{{60, 50, "ghi"}, {40, 30, "def"}, {20, 10, "abc"}}, "xyz"}));
    TEST(map.empty());
    TEST_EQUAL(map.default_value(), "xyz");
    TEST_EQUAL(std::format("{}", map), R"({default:"xyz"})");
    TEST(! map.contains(1));
    TEST_EQUAL(map[1], "xyz");

}
