/*!
 * \file video_reader.hpp
 *
 * \brief 视频讀取接口类
 * \author yuewu,cyy
 */

#pragma once

#include <array>
#include <optional>
#include <string>

#include "frame.hpp"

namespace cyy::naive_lib::video {

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
  //! \note 如果是指定本地文件，则加上file://
  //! \note 先关闭之前打开的视频再打开此视频
  virtual bool open(const std::string &url) = 0;

  //! \brief 關閉视频
  virtual void close() noexcept = 0;

  //! \brief 获取视频帧率
  //! \note 如果无法获取视频帧率，則返回空
  virtual std::optional<std::array<size_t, 2>> get_frame_rate() = 0;

  //! \brief 获取下一帧
  //! \return first>0 成功
  //	      first=0 EOF
  //	      first<0 失敗
  //	如果first<=0，返回空内容
  virtual std::pair<int, frame> next_frame() = 0;

    //! \brief jump to a frame
    virtual bool seek_frame(size_t frame_seq)=0;
    
};
} // namespace cyy::naive_lib::video
