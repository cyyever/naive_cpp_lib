/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>
#include <spdlog/common.h>

#include "log/log.hpp"

TEST_CASE("default console logger") {
  cyy::naive_lib::log::set_thread_name("test_thread");

  LOG_ERROR("hello world.....error");
  LOG_INFO("hello world.....info");
  LOG_DEBUG("hello world.....debug");
  LOG_WARN("hello world.....warn");
}

TEST_CASE("set logger level") {
  cyy::naive_lib::log::set_level(spdlog::level::level_enum::err);
  LOG_WARN("no warns");
  LOG_ERROR("an error");
}

TEST_CASE("check format string") { LOG_ERROR("{} {}"); }

TEST_CASE("file logger") {
  cyy::naive_lib::log::setup_file_logger(".", "file_logger",
                                         spdlog::level::level_enum::info);

  LOG_ERROR("hello world.....error");
  LOG_INFO("hello world.....info");
  LOG_DEBUG("hello world.....debug");
  LOG_WARN("hello world.....warn");
}
