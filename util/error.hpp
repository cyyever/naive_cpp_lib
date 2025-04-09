/*!
 * \file error.hpp
 *
 * \brief 封装一些错误处理逻辑
 * \author cyy
 * \date 2017-01-17
 */

#pragma once

#include <cerrno>

#ifdef _WIN32
#include <windows.h>

#include <strsafe.h>
#endif
import std;

namespace cyy::naive_lib::util {

  //! \brief 获取errno的字符串描述
  //! \note 本函数线程安全
  std::string errno_to_str(int errno_);
  inline std::string errno_to_str() { return errno_to_str(errno); }

#ifdef _WIN32
  //! \brief 获取Windows API errno的字符串描述
  std::string get_winapi_error_msg(DWORD winapi_errno);
#endif

} // namespace cyy::naive_lib::util
