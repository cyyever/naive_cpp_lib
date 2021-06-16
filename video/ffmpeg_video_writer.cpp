/*!
 * \file ffmpeg_writer.cpp
 *
 * \brief
 * 封装ffmpeg对视频流的寫操作，參考ffmpeg源碼目錄下的doc/examples/muxing.c文件
 * \author Yue Wu,cyy
 */

#include "ffmpeg_video_writer.hpp"

#include "ffmpeg_video_writer_impl.hpp"

namespace cyy::naive_lib::video {

  ffmpeg_writer::ffmpeg_writer()
      : pimpl{std::make_unique<ffmpeg_writer_impl>()} {}
  ffmpeg_writer::~ffmpeg_writer() = default;

  bool ffmpeg_writer::open(const std::string &url,
                           const std::string &format_name, int video_width,
                           int video_height,
                           std::optional<std::pair<int, int>> frame_rate) {
    return pimpl->open(url, format_name, video_width, video_height, frame_rate);
  }

  bool ffmpeg_writer::write_frame(const cv::Mat &frame_mat) {
    return pimpl->write_frame(frame_mat);
  }

  void ffmpeg_writer::close() noexcept { pimpl->close(); }
  const std::string &ffmpeg_writer::get_url() const { return pimpl->get_url(); }
} // namespace cyy::naive_lib::video
