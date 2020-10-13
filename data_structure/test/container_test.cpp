/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <chrono>
#include <functional>
#include <thread>

#include <doctest/doctest.h>

#include "data_structure/thread_safe_container.hpp"
#include <type_traits>

TEST_CASE("thread_safe_linear_container") {
  cyy::cxx_lib::thread_safe_linear_container<std::vector<int>> container;

  CHECK(container.const_ref()->empty());

  SUBCASE("push_back") {
    container.push_back(1);
    CHECK(!container.const_ref()->empty());
    CHECK_GT(container.const_ref()->size(), 0);
    container.clear();
    CHECK(container.const_ref()->empty());
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
    container.clear();
    CHECK(container.const_ref()->empty());
  }

  SUBCASE("back") {
    using namespace std::chrono_literals;
    container.clear();
    container.push_back(1);
    auto val = container.back(1s);
    CHECK(val.has_value());
    CHECK_EQ(val.value(), 1);
    container.pop_front();
    container.push_back(2);
    val = container.pop_front(1us);
    CHECK(val.has_value());
    CHECK_EQ(val.value(), 2);
    val = container.back(1us);
    CHECK(!val.has_value());
    container.clear();
    CHECK(container.const_ref()->empty());
  }
  SUBCASE("concurrent pop_front") {
    std::vector<std::thread> thds;
    for (int i = 0; i < 10; i++) {
      thds.emplace_back([&container]() {
        CHECK(!container.pop_front(std::chrono::microseconds(1)).has_value());
      });
    }
    for (auto &thd : thds) {
      thd.join();
    }
  }
}
