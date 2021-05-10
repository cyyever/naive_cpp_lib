/*!
 * \file writer.hpp
 *
 * \brief 视频讀取接口类
 * \author yuewu,cyy
 */

#pragma once

#include <string>

#include "frame.hpp"

namespace cyy::naive_lib::video {

  //! brief 视频讀取接口类
  class writer {
  public:
    writer() = default;

    writer(const writer &) = delete;
    writer &operator=(const writer &) = delete;

    writer(writer &&) = default;
    writer &operator=(writer &&) = default;

    virtual ~writer() = default;

    //! \brief 打開视频
    //! \param url 视频地址
    //! \note 如果是指定本地文件，则加上file://
    //! \note 先关闭之前打开的视频再打开此视频
    virtual bool open(const std::string &url, const std::string &format_name,
                      int video_width, int video_height) = 0;

    //! \brief 寫入一幀
    virtual bool write_frame(const cv::Mat &mat) = 0;

    //! \brief 關閉视频
    virtual void close() = 0;
    virtual const std::string get_url() const=0;
  };
} // namespace cyy::naive_lib::video
