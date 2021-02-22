/*!
 * \file ffmpeg_reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_video_reader.hpp"

#include "ffmpeg_video_reader_impl.hpp"

namespace cyy::naive_lib::video {

  ffmpeg_reader::ffmpeg_reader()
      : pimpl{std::make_unique<ffmpeg_reader_impl<true>>()} {}
  ffmpeg_reader::~ffmpeg_reader() = default;
  bool ffmpeg_reader::open(const std::string &url) { return pimpl->open(url); }

  //! \brief 获取下一帧
  //! \note 如果失败，返回空内容
  std::pair<int, frame> ffmpeg_reader::next_frame() {
    return pimpl->next_frame();
  }

  //! \brief 获取視頻寬
  std::optional<int> ffmpeg_reader::get_video_width() const {
    return pimpl->get_video_width();
  }
  //! \brief 获取視頻高
  std::optional<int> ffmpeg_reader::get_video_height() const {
    return pimpl->get_video_height();
  }

  std::optional<std::array<size_t, 2>> ffmpeg_reader::get_frame_rate() {
    return pimpl->get_frame_rate();
  }

  void
  ffmpeg_reader::set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
    pimpl->set_play_frame_rate(frame_rate);
  }

  void ffmpeg_reader::keep_non_key_frames() { pimpl->keep_non_key_frames(); }
  void ffmpeg_reader::drop_non_key_frames() { pimpl->drop_non_key_frames(); }
  void ffmpeg_reader::close() { pimpl->close(); }
  bool ffmpeg_reader::seek_frame(size_t frame_seq) {
    return pimpl->seek_frame(frame_seq);
  }
} // namespace cyy::naive_lib::video
