/*!
 * \file ffmpeg_base.hpp
 *
 * \brief 封装ffmpeg庫相關函數
 * \author Yue Wu,cyy
 */

#pragma once

#include <string>

//! \brief 封装ffmpeg库相關函數
namespace cyy::naive_lib::video {
  class ffmpeg_base {
  public:
    ffmpeg_base();
    virtual ~ffmpeg_base() { close(); }

    bool has_open() const { return opened; }
    bool is_live_stream() const { return url_scheme == "rtsp"; }

  protected:
    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    virtual bool open(const std::string &url);
    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    virtual void close() { opened = false; }
    std::string errno_to_str(int err);

  protected:
    bool opened{false};
    std::string url_scheme;
  };
} // namespace cyy::naive_lib::video
