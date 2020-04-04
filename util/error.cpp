/*!
 * \file error.cpp
 *
 * \brief 封装一些错误处理逻辑
 * \author cyy
 * \date 2017-01-17
 */

#include <mutex>
#include <system_error>

#include "error.hpp"

namespace cyy::cxx_lib::util {

  //! \brief 获取errno的字符串描述
  //! \note 本函数线程安全
  std::string errno_to_str(int errno_) {
    static std::mutex err_mutex;

    std::lock_guard lk(err_mutex);

    auto msg = std::system_category().default_error_condition(errno_).message();
    if (msg.empty()) {
      return std::string("Unknown error ") + std::to_string(errno_);
    }
    return msg;
  }

#ifdef _WIN32
  //! \brief 获取Windows API errno的字符串描述
  std::string get_winapi_error_msg(DWORD winapi_errno) {
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;

    if (!FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, winapi_errno, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPTSTR)&lpMsgBuf, 0, NULL)) {
      return {};
    }

    std::string msg((char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return msg;
  }
#endif

} // namespace cyy::cxx_lib::util
