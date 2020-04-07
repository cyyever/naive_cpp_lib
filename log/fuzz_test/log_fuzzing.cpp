/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include "log/log.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  try {
    if (Size == 0) {
      return 0;
    }
    std::string fmt(reinterpret_cast<const char *>(Data), Size / 2);
    std::string msg(reinterpret_cast<const char *>(Data + Size / 2),
                    Size - Size / 2);
    LOG_ERROR(fmt.c_str(), msg);
  } catch (...) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
