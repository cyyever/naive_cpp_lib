/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <chrono>
#include <doctest/doctest.h>
#include <functional>
#include <thread>

#include "util/thread_safe_container.hpp"

TEST_CASE("thread_safe_linear_container") {
  cyy::cxx_lib::thread_safe_linear_container<std::vector<int>> container;

  CHECK(container.const_ref()->empty());

  SUBCASE("push_back") {
    container.push_back(1);
    CHECK(!container.const_ref()->empty());
    CHECK_GT(container.const_ref()->size(), 0);
  }

  SUBCASE("concurrent_push_back") {
    container.clear();
    std::vector<std::thread> thds;
    for (int i = 0; i < 10; i++) {
      thds.emplace_back([i, &container]() { container.push_back(i); });
    }
    for (auto &thd : thds) {
      thd.join();
    }
    CHECK_EQ(container.const_ref()->size(), 10);
  }

  SUBCASE("front") {
    using namespace std::chrono_literals;
    container.clear();
    container.push_back(1);
    auto val = container.back(1s);
    CHECK(val.has_value());
    CHECK_EQ(val.value(), 1);
  }
}
