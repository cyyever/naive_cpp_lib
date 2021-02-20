/*!
 * \file string.hpp
 *
 * \brief 封装一些字符串操作
 * \author cyy
 * \date 2017-01-17
 */
#pragma once

#include <string>

namespace cyy::naive_lib::strings {

#ifdef _WIN32
  std::string GBK_to_UTF8(const std::string &str);
  std::string UTF16_to_UTF8(const char *str, size_t len);
  std::string UTF8_to_GBK(const std::string &str);
#endif
} // namespace cyy::naive_lib::strings
