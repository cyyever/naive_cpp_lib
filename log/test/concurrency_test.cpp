/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#include "log/log.hpp"

TEST_CASE("concurrency") {
  cyy::naive_lib::log::setup_file_logger(".", "file_logger",
                                         spdlog::level::level_enum::info);
  std::vector<std::jthread> thds;

  const size_t thread_num = 100;
  thds.reserve(thread_num);
  for (size_t i = 0; i < thread_num; i++) {
    thds.emplace_back([=]() {
      LOG_ERROR("{} {} {}", "hello", "world", i);
      LOG_WARN("{} {} {}", "hello", "world", i);
    });
  }
}
