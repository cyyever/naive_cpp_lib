/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include "data_structure/ordered_dict.hpp"

#include <doctest/doctest.h>

TEST_CASE("ordered_dict") {
  cyy::cxx_lib::ordered_dict<int, std::string> container;

  CHECK(container.empty());

  SUBCASE("emplace") {
    container.emplace(1, "");
    CHECK(!container.empty());
    CHECK_EQ(container.size(), 1);
    container.emplace(1, "hello");
    CHECK_EQ(container.size(), 1);
    CHECK_EQ(*container.rbegin(), "hello");
    container.clear();
    CHECK(container.empty());
  }

  SUBCASE("find") {
    CHECK_EQ(container.find(1), container.end());
    CHECK_EQ(container.begin(), container.end());
    CHECK_EQ(container.rbegin(), container.rend());

    container.emplace(1, "a");
    container.emplace(2, "b");
    auto it = container.begin();
    CHECK_EQ(*it, "a");
    it++;
    CHECK_EQ(*it, "b");

    container.find(1);
    it = container.begin();
    CHECK_EQ(*it, "b");
    it++;
    CHECK_EQ(*it, "a");
    container.clear();
  }

  SUBCASE("pop_front") {
    container.emplace(1, "a");
    container.emplace(2, "b");
    CHECK(!container.empty());
    CHECK_EQ(container.size(), 2);
    auto [k, v] = container.pop_front();
    CHECK_EQ(k, 1);
    CHECK_EQ(v, "a");
    std::tie(k, v) = container.pop_front();
    CHECK_EQ(k, 2);
    CHECK_EQ(v, "b");
    CHECK(container.empty());
    CHECK_THROWS(container.pop_front());
  }

  SUBCASE("no move_to_end") {
    container.move_to_end_in_update = false;
    container.emplace(1, "a");
    container.emplace(2, "b");
    container.emplace(1, "c");
    CHECK_EQ(container.size(), 2);

    auto [k, v] = container.pop_front();
    CHECK_EQ(k, 1);
    CHECK_EQ(v, "c");
    std::tie(k, v) = container.pop_front();
    CHECK_EQ(k, 2);
    CHECK_EQ(v, "b");
    container.move_to_end_in_update = true;
    container.clear();
  }
  SUBCASE("no find_to_end") {
    container.move_to_end_in_finding = false;
    container.emplace(1, "a");
    container.emplace(2, "b");
    container.find(1);

    auto [k, v] = container.pop_front();
    CHECK_EQ(k, 1);
    CHECK_EQ(v, "a");
    std::tie(k, v) = container.pop_front();
    CHECK_EQ(k, 2);
    CHECK_EQ(v, "b");
    container.move_to_end_in_finding = true;
  }
}
