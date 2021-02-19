/*!
 * \file ffmpeg_writer.cpp
 *
 * \brief
 * 封装ffmpeg对视频流的寫操作，參考ffmpeg源碼目錄下的doc/examples/muxing.c文件
 * \author Yue Wu,cyy
 */

#include "ffmpeg_writer.hpp"

#include "ffmpeg_writer_impl.hpp"

namespace cyy::naive_lib::video {

  ffmpeg_writer::ffmpeg_writer()
      : pimpl{std::make_unique<ffmpeg_writer_impl>()} {}
  ffmpeg_writer::~ffmpeg_writer() = default;

  bool ffmpeg_writer::open(const std::string &url,
                           const std::string &format_name, int video_width,
                           int video_height) {
    return pimpl->open(url, format_name, video_width, video_height);
  }

  bool ffmpeg_writer::write_frame(const cv::Mat &frame_mat) {
    return pimpl->write_frame(frame_mat);
  }

  void ffmpeg_writer::close() { pimpl->close(); }
} // namespace cyy::naive_lib::video
