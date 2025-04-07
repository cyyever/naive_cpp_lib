/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#ifdef _WIN32
#include "util/string.hpp"
TEST_CASE("UTF8_and_GBK_conv") {
  std::string test_str = "a123";
  CHECK(cyy::naive_lib::strings::GBK_to_UTF8(
            cyy::naive_lib::strings::UTF8_to_GBK(test_str)) == test_str);
}
#endif
