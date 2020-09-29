/*!
 * \file audio_reader.hpp
 *
 * \brief sound频讀取接口类
 * \author yuewu,cyy
 */

#pragma once

#include <array>
#include <chrono>
#include <optional>
#include <string>

namespace deepir::audio {

//! brief 视频讀取接口类
class reader {
public:
  reader() = default;

  reader(const reader &) = delete;
  reader &operator=(const reader &) = delete;

  reader(reader &&) = default;
  reader &operator=(reader &&) = default;

  virtual ~reader() = default;

  //! \brief 打開视频
  //! \param url 视频地址
  //! \note 先关闭之前打开的视频再打开此视频
  virtual bool open(const std::string &url) = 0;

  //! \brief 關閉视频
  virtual void close() = 0;

  virtual std::optional<std::chrono::milliseconds> get_duration() = 0;
};
} // namespace deepir::audio
