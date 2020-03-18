/*!
 * \file log_test.cpp
 *
 * \brief 测试log函数
 * \author cyy
 * \date 2017-01-17
 */

#include "../src/log.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  try{
    std::string msg(reinterpret_cast<const char*>(Data),Size);
    LOG_ERROR(msg.c_str());
    LOG_INFO(msg.c_str());
    LOG_DEBUG(msg.c_str());
    LOG_WARN(msg.c_str());
  }catch(...){
  }
  return 0; // Non-zero return values are reserved for future use.
}
