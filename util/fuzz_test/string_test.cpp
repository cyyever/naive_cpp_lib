/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <cstdint>

#include "util/string.hpp"
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  cyy::cxx_lib::strings::split({(const char *)Data, Size}, ' ');
  return 0; // Non-zero return values are reserved for future use.
}
