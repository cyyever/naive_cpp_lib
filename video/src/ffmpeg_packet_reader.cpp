/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_packet_reader.hpp"

#include "ffmpeg_video_reader_impl.hpp"

namespace cyy::naive_lib::video::ffmpeg {

  packet_reader::packet_reader()
      : pimpl{std::make_unique<reader_impl<false>>()} {}
  packet_reader::~packet_reader() = default;
  bool packet_reader::open(const std::string &url) { return pimpl->open(url); }

  std::optional<std::array<size_t, 2>> packet_reader::get_frame_rate() {
    return pimpl->get_frame_rate();
  }

  void packet_reader::close() { pimpl->close(); }

  void
  packet_reader::set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
    return pimpl->set_play_frame_rate(frame_rate);
  }
  std::pair<int, std::shared_ptr<AVPacket>> packet_reader::next_packet() {
    return pimpl->next_packet();
  }

} // namespace cyy::naive_lib::video::ffmpeg
