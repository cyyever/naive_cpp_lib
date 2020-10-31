/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include <thread>

#include <doctest/doctest.h>

#include "log/log.hpp"

TEST_CASE("concurrency") {
  cyy::naive_lib::log::setup_file_logger(".", "file_logger",
                                       spdlog::level::level_enum::info);
  std::vector<std::thread> thds;

  for (int i = 0; i < 100; i++) {
    thds.emplace_back([=]() {
      LOG_ERROR("{} {} {}", "hello", "world", i);
      LOG_WARN("{} {} {}", "hello", "world", i);
    });
  }

  for (auto &thd : thds) {
    thd.join();
  }
}
