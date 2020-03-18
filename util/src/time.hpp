/*!
 * \file time.hpp
 *
 * \brief 封装一些时间函数
 * \author cyy
 * \date 2016-04-18
 */

#pragma once

#include <chrono>
#include <cstdint>

namespace cyy::cxx::time {
  /// \brief  get current time, in milliseconds
  /// \return milliseconds
  /// \note
  /// 我们之所以写这个函数是因为在简单的情境下，c++原生的调用太啰嗦了，如果要细分c++标准库提供的时钟类型，那么应该写原生代码
  template <typename ClockType = std::chrono::system_clock> uint64_t now_ms() {
    return static_cast<uint64_t>(
        std::chrono::time_point_cast<std::chrono::milliseconds>(
            ClockType::now())
            .time_since_epoch()
            .count());
  }
} // namespace cyy::cxx::time
