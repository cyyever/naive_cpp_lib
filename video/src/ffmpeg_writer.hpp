/*!
 * \file ffmpeg_writer.h
 *
 * \brief 封装ffmpeg对视频流的讀操作
 * \author Yue Wu,cyy
 */
#pragma once

#include "frame.hpp"
#include "writer.hpp"
#include <memory>

namespace cyy::cxx_lib::video::ffmpeg {

  //! \brief 封装ffmpeg对视频流的讀操作
  class writer final : public ::cyy::cxx_lib::video::writer {
  public:
    writer();

    ~writer() override;

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url, const std::string &format_name,
              int video_width, int video_height) override;

    //! \brief 寫入一幀
    bool write_frame(const cv::Mat &frame_mat) override;

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() override;

  public:
    class impl;

  private:
    std::unique_ptr<impl> pimpl;
  };
} // namespace cyy::cxx_lib::video::ffmpeg
