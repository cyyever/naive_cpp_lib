/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#include "../src/string.hpp"

TEST_CASE("split") {
  auto tmp = deepir::strings::split(" a b ", ' ');

  CHECK(tmp.size() == 2);
  CHECK(tmp[0] == "a");
  CHECK(tmp[1] == "b");
}

TEST_CASE("has_suffix") {
  CHECK(!deepir::strings::has_suffix("abc", "e"));
  CHECK(!deepir::strings::has_suffix("abc", "ab"));
  CHECK(deepir::strings::has_suffix("abc", "bc"));
}

#ifdef _WIN32
TEST_CASE("UTF8_and_GBK_conv") {
  std::string test_str = "a123";
  CHECK(deepir::strings::GBK_to_UTF8(deepir::strings::UTF8_to_GBK(test_str)) ==
        test_str);
}
#endif
