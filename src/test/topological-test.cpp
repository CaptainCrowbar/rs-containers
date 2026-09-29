#include "rs-containers/topological.hpp"
#include "rs-core/unit-test.hpp"
#include <concepts>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <vector>

using namespace RS;
using namespace RS::Containers;
using namespace RS::Containers::Detail;

namespace RS::Containers::Detail {

    template <std::regular T, std::strict_weak_order<T, T> Compare>
    std::string format_by_node(TopologicalOrder<T, Compare> topo) {
        std::string out;
        for (auto& node: topo.graph_) {
            out += std::format("{}", node.first);
            if (! node.second.right.empty()) {
                out += std::format(" => {}", node.second.right);
            }
            out += "\n";
        }
        return out;
    }

    template <std::regular T, std::strict_weak_order<T, T> Compare>
    std::string format_by_set(TopologicalOrder<T, Compare> topo) {
        std::string out;
        while (! topo.empty()) {
            out += std::format("{}\n", topo.pop_front_set());
        }
        return out;
    }

}

void test_rs_containers_topological_sorting() {

    TopologicalOrder<int> topo;
    std::vector<int> v;
    std::string s;
    std::optional<int> n;

    TEST(topo.empty());
    TEST_EQUAL(topo.size(), 0u);
    TEST_EQUAL(format_by_node(topo), "");
    TEST_EQUAL(format_by_set(topo), "");

    TRY(topo.insert(1));
    TEST(! topo.empty());
    TEST_EQUAL(topo.size(), 1u);
    TEST_EQUAL(format_by_node(topo), "1\n");
    TEST_EQUAL(format_by_set(topo), "[1]\n");

    TRY(topo.insert(2, 3));
    TEST_EQUAL(topo.size(), 3u);
    TEST_EQUAL(format_by_node(topo),
        "1\n"
        "2 => {3}\n"
        "3\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2]\n"
        "[3]\n"
    );

    TRY(topo.insert(4, 5));
    TEST_EQUAL(topo.size(), 5u);
    TEST_EQUAL(format_by_node(topo),
        "1\n"
        "2 => {3}\n"
        "3\n"
        "4 => {5}\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2, 4]\n"
        "[3, 5]\n"
    );

    TRY(topo.insert(2, 4));
    TEST_EQUAL(topo.size(), 5u);
    TEST_EQUAL(format_by_node(topo),
        "1\n"
        "2 => {3, 4}\n"
        "3\n"
        "4 => {5}\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2]\n"
        "[3, 4]\n"
        "[5]\n"
    );

    TRY(topo.insert(1, 4));
    TEST_EQUAL(topo.size(), 5u);
    TEST_EQUAL(format_by_node(topo),
        "1 => {4}\n"
        "2 => {3, 4}\n"
        "3\n"
        "4 => {5}\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2]\n"
        "[3, 4]\n"
        "[5]\n"
    );

    TRY(topo.clear());
    TEST_EQUAL(topo.size(), 0u);
    TEST_EQUAL(format_by_node(topo), "");
    TEST_EQUAL(format_by_set(topo), "");

    TRY(topo.insert({1,2,3}, {4,5,6}));
    TEST_EQUAL(topo.size(), 6u);

    TEST(! topo.contains(0));  TEST(! topo.is_front(0));  TEST(! topo.is_back(0));
    TEST(topo.contains(1));    TEST(topo.is_front(1));    TEST(! topo.is_back(1));
    TEST(topo.contains(2));    TEST(topo.is_front(2));    TEST(! topo.is_back(2));
    TEST(topo.contains(3));    TEST(topo.is_front(3));    TEST(! topo.is_back(3));
    TEST(topo.contains(4));    TEST(! topo.is_front(4));  TEST(topo.is_back(4));
    TEST(topo.contains(5));    TEST(! topo.is_front(5));  TEST(topo.is_back(5));
    TEST(topo.contains(6));    TEST(! topo.is_front(6));  TEST(topo.is_back(6));
    TEST(! topo.contains(7));  TEST(! topo.is_front(7));  TEST(! topo.is_back(7));

    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[1, 2, 3]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[4, 5, 6]");

    TEST_EQUAL(format_by_node(topo),
        "1 => {4, 5, 6}\n"
        "2 => {4, 5, 6}\n"
        "3 => {4, 5, 6}\n"
        "4\n"
        "5\n"
        "6\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2, 3]\n"
        "[4, 5, 6]\n"
    );

    TRY(topo.clear());
    TRY(topo.insert(1, 2, 3, 4, 5));
    TEST_EQUAL(topo.size(), 5u);

    TEST(! topo.contains(0));  TEST(! topo.is_front(0));  TEST(! topo.is_back(0));
    TEST(topo.contains(1));    TEST(topo.is_front(1));    TEST(! topo.is_back(1));
    TEST(topo.contains(2));    TEST(! topo.is_front(2));  TEST(! topo.is_back(2));
    TEST(topo.contains(3));    TEST(! topo.is_front(3));  TEST(! topo.is_back(3));
    TEST(topo.contains(4));    TEST(! topo.is_front(4));  TEST(! topo.is_back(4));
    TEST(topo.contains(5));    TEST(! topo.is_front(5));  TEST(topo.is_back(5));
    TEST(! topo.contains(6));  TEST(! topo.is_front(6));  TEST(! topo.is_back(6));

    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[1]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[5]");

    TEST_EQUAL(format_by_node(topo),
        "1 => {2, 3, 4, 5}\n"
        "2 => {3, 4, 5}\n"
        "3 => {4, 5}\n"
        "4 => {5}\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1]\n"
        "[2]\n"
        "[3]\n"
        "[4]\n"
        "[5]\n"
    );

    TRY(topo.clear());
    v = {1, 2, 3};  TRY(topo.insert(v));
    v = {1, 4, 5};  TRY(topo.insert(v));
    v = {2, 3, 6};  TRY(topo.insert(v));
    v = {4, 5, 6};  TRY(topo.insert(v));
    TEST_EQUAL(topo.size(), 6u);

    TEST_EQUAL(format_by_set(topo),
        "[1]\n"
        "[2, 4]\n"
        "[3, 5]\n"
        "[6]\n"
    );

    TRY(topo.clear());
    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[]");
    TRY(n = topo.front());      TEST(! n);
    TRY(n = topo.back());       TEST(! n);

    TRY(topo.insert(1));
    TRY(topo.insert(2, 3));
    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[1, 2]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[1, 3]");
    TRY(n = topo.front());      TEST(n); TEST_EQUAL(n.value(), 1);
    TRY(n = topo.back());       TEST(n); TEST_EQUAL(n.value(), 1);
    TEST(! topo.erase(0));
    TEST(topo.erase(1));
    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[2]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[3]");
    TRY(n = topo.front());      TEST(n); TEST_EQUAL(n.value(), 2);
    TRY(n = topo.back());       TEST(n); TEST_EQUAL(n.value(), 3);
    TEST(topo.erase(2));
    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[3]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[3]");
    TRY(n = topo.front());      TEST(n); TEST_EQUAL(n.value(), 3);
    TRY(n = topo.back());       TEST(n); TEST_EQUAL(n.value(), 3);
    TEST(topo.erase(3));
    TEST(topo.empty());

    TRY(topo.clear());
    TRY(topo.insert(1, {3,4}));
    TRY(topo.insert(2, {3,5}));
    TRY(topo.insert(3, {4,5}));
    TEST_EQUAL(format_by_node(topo),
        "1 => {3, 4}\n"
        "2 => {3, 5}\n"
        "3 => {4, 5}\n"
        "4\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2]\n"
        "[3]\n"
        "[4, 5]\n"
    );

    TEST(topo.erase(3));
    TEST_EQUAL(format_by_node(topo),
        "1 => {4}\n"
        "2 => {5}\n"
        "4\n"
        "5\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[1, 2]\n"
        "[4, 5]\n"
    );

    TRY(topo.clear());
    TRY(topo.insert(1));
    TRY(topo.insert(2, 3));
    TRY(n = topo.pop_front());  TEST(n);  TEST_EQUAL(n.value(), 1);
    TRY(n = topo.pop_front());  TEST(n);  TEST_EQUAL(n.value(), 2);
    TRY(n = topo.pop_front());  TEST(n);  TEST_EQUAL(n.value(), 3);
    TEST(topo.empty());

    TRY(topo.clear());
    TRY(topo.insert(1));
    TRY(topo.insert(2, 3));
    TRY(n = topo.pop_back());  TEST(n);  TEST_EQUAL(n.value(), 1);
    TRY(n = topo.pop_back());  TEST(n);  TEST_EQUAL(n.value(), 3);
    TRY(n = topo.pop_back());  TEST(n);  TEST_EQUAL(n.value(), 2);
    TEST(topo.empty());

    TRY(topo.clear());
    TRY(topo.insert(1));
    TRY(topo.insert(2, 3));
    TRY(v = topo.pop_front_set());  TEST_EQUAL(std::format("{}", v), "[1, 2]");
    TRY(v = topo.pop_front_set());  TEST_EQUAL(std::format("{}", v), "[3]");
    TEST(topo.empty());
    TRY(v = topo.pop_front_set());  TEST_EQUAL(std::format("{}", v), "[]");

    TRY(topo.clear());
    TRY(topo.insert(1));
    TRY(topo.insert(2, 3));
    TRY(v = topo.pop_back_set());  TEST_EQUAL(std::format("{}", v), "[1, 3]");
    TRY(v = topo.pop_back_set());  TEST_EQUAL(std::format("{}", v), "[2]");
    TEST(topo.empty());
    TRY(v = topo.pop_back_set());  TEST_EQUAL(std::format("{}", v), "[]");

    TRY(topo.clear());
    TRY(topo.insert(1, 2));
    TRY(topo.insert(2, 3));
    TRY(topo.insert(3, 4));
    TRY(topo.insert(4, 2));
    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[1]");
    TRY(n = topo.pop_front());  TEST(n); TEST_EQUAL(n.value(), 1);
    TEST_THROW(topo.front_set(), TopologicalCycle, "Dependency cycle in topological sort");
    TEST_THROW(topo.back_set(), TopologicalCycle, "Dependency cycle in topological sort");
    TEST_THROW(topo.pop_front(), TopologicalCycle, "Dependency cycle in topological sort");
    TEST_THROW(topo.pop_front_set(), TopologicalCycle, "Dependency cycle in topological sort");
    TEST_THROW(topo.pop_back(), TopologicalCycle, "Dependency cycle in topological sort");
    TEST_THROW(topo.pop_back_set(), TopologicalCycle, "Dependency cycle in topological sort");

}

void test_rs_containers_topological_sorting_reverse() {

    TopologicalOrder<int, std::greater<>> topo;
    std::vector<int> v;
    std::string s;

    TRY(topo.insert({1,2,3}, {4,5,6}));
    TEST_EQUAL(topo.size(), 6u);

    TEST(! topo.contains(0));  TEST(! topo.is_front(0));  TEST(! topo.is_back(0));
    TEST(topo.contains(1));    TEST(topo.is_front(1));    TEST(! topo.is_back(1));
    TEST(topo.contains(2));    TEST(topo.is_front(2));    TEST(! topo.is_back(2));
    TEST(topo.contains(3));    TEST(topo.is_front(3));    TEST(! topo.is_back(3));
    TEST(topo.contains(4));    TEST(! topo.is_front(4));  TEST(topo.is_back(4));
    TEST(topo.contains(5));    TEST(! topo.is_front(5));  TEST(topo.is_back(5));
    TEST(topo.contains(6));    TEST(! topo.is_front(6));  TEST(topo.is_back(6));
    TEST(! topo.contains(7));  TEST(! topo.is_front(7));  TEST(! topo.is_back(7));

    TRY(v = topo.front_set());  TEST_EQUAL(std::format("{}", v), "[3, 2, 1]");
    TRY(v = topo.back_set());   TEST_EQUAL(std::format("{}", v), "[6, 5, 4]");

    TEST_EQUAL(format_by_node(topo),
        "6\n"
        "5\n"
        "4\n"
        "3 => {6, 5, 4}\n"
        "2 => {6, 5, 4}\n"
        "1 => {6, 5, 4}\n"
    );
    TEST_EQUAL(format_by_set(topo),
        "[3, 2, 1]\n"
        "[6, 5, 4]\n"
    );

}
