/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#include "../src/log.hpp"

TEST_CASE("default console logger") {
  LOG_ERROR("hello world.....error");
  LOG_INFO("hello world.....info");
  LOG_DEBUG("hello world.....debug");
  LOG_WARN("hello world.....warn");
}

TEST_CASE("single message") { LOG_ERROR("{} {}"); }

TEST_CASE("file logger") {
  cyy::cxx::log::setup_file_logger(".", "file_logger",
                                 spdlog::level::level_enum::info);

  LOG_ERROR("hello world.....error");
  LOG_INFO("hello world.....info");
  LOG_DEBUG("hello world.....debug");
  LOG_WARN("hello world.....warn");
}
