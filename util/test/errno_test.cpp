/*!
 * \file errno_test.cpp
 *
 * \brief 測試錯誤號相關函數
 * \author cyy
 * \date 2017-12-29
 */

#include <doctest/doctest.h>

#include "../src/error.hpp"

#ifndef _WIN32
TEST_CASE("errno_to_str") {
  for (int err = 0; err <= EACCES; err++) {
    CHECK_EQ(cyy::cxx_lib::util::errno_to_str(err), std::string(strerror(err)));
  }
}
#endif

#ifdef _WIN32
TEST_CASE("get_winapi_error_msg") {
  for (int err = 0; err < 35; err++) {
    auto msg = cyy::cxx_lib::util::get_winapi_error_msg(err);
    CHECK(!msg.empty());
  }
}
#endif
