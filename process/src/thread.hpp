/*!
 * \file thread.hpp
 *
 * \brief 封裝thread處理代碼
 * \author cyy
 */

#pragma once

#include <chrono>
#include <optional>
#include <signal.h>

#include <sys/signalfd.h>
#include <sys/types.h>

namespace cyy::cxx_lib::this_thread {
#ifndef _WIN32

  /// \brief 阻塞指定時間讀取信號
  /// \note caller必須保證信號已經被阻塞
  std::optional<signalfd_siginfo>
  read_signal(const sigset_t &set,
              const std::optional<std::chrono::milliseconds> &timeout = {});
#endif
} // namespace cyy::cxx_lib::this_thread
