/*!
 * \file string.hpp
 *
 * \brief 封装一些字符串操作
 * \author cyy
 * \date 2017-01-17
 */
#pragma once

#include <string>
#include <vector>

namespace cyy::cxx_lib::strings {

  /// \brief split string by delimiter
  std::vector<std::string> split(const std::string &s, char c);


#ifdef _WIN32
  std::string GBK_to_UTF8(const std::string &str);
  std::string UTF16_to_UTF8(const char *str, size_t len);
  std::string UTF8_to_GBK(const std::string &str);
#endif
} // namespace cyy::cxx_lib::strings
