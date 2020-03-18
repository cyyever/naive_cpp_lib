/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <doctest/doctest.h>

#include "../src/ordered_dict.hpp"

TEST_CASE("ordered_dict") {
  cyy::cxx_lib::ordered_dict<int, std::string> container;

  CHECK(container.empty());

  SUBCASE("emplace") {
    container.emplace(1, "");
    CHECK(!container.empty());
    CHECK_EQ(container.size(), 1);
  }

  SUBCASE("find") {
    container.clear();
    container.emplace(1, "hello");
    auto it = container.find(1);
    CHECK_NE(it, container.end());
    CHECK_EQ(it->second, "hello");
  }
}
