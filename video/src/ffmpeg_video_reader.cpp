/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_video_reader.hpp"

#include "ffmpeg_video_reader_impl.hpp"

namespace cyy::naive_lib::video::ffmpeg {

  reader::reader() : pimpl{std::make_unique<reader_impl<true>>()} {}
  reader::~reader() = default;
  bool reader::open(const std::string &url) { return pimpl->open(url); }

  //! \brief 获取下一帧
  //! \note 如果失败，返回空内容
  std::pair<int, frame> reader::next_frame() { return pimpl->next_frame(); }

  std::optional<std::array<size_t, 2>> reader::get_frame_rate() {
    return pimpl->get_frame_rate();
  }

  void reader::set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
    pimpl->set_play_frame_rate(frame_rate);
  }

  void reader::drop_non_key_frames() { pimpl->drop_non_key_frames(); }
  void reader::close() { pimpl->close(); }

} // namespace cyy::naive_lib::video::ffmpeg
