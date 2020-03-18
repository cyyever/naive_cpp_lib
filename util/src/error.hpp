/*!
 * \file error.hpp
 *
 * \brief 封装一些错误处理逻辑
 * \author cyy
 * \date 2017-01-17
 */

#pragma once

#include <string>

#ifdef _WIN32
#include <strsafe.h>
#include <windows.h>
#endif

namespace cyy::cxx::util {

//! \brief 获取errno的字符串描述
//! \note 本函数线程安全
std::string errno_to_str(int errno_);

#ifdef _WIN32
//! \brief 获取Windows API errno的字符串描述
std::string get_winapi_error_msg(DWORD winapi_errno);
#endif

} // namespace cyy::cxx::util
