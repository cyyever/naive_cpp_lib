/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#include "../src/log.hpp"

TEST_CASE("custom console logger") {
  cyy::cxx::log::console_logger_name = "custom name";
  LOG_ERROR("hello world.....error");
  LOG_INFO("hello world.....info");
  LOG_DEBUG("hello world.....debug");
  LOG_WARN("hello world.....warn");
}
