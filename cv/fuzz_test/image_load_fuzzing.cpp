/*!
 * \file image_test.cpp
 *
 * \brief 测试image函数
 * \author cyy
 * \date 2017-01-17
 */

#include "cv/mat.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  try {
    cyy::naive_lib::opencv::mat::load(Data, Size);
  } catch (...) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
