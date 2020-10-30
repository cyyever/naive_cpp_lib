/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_reader.hpp"

#include "ffmpeg_reader_impl.hpp"

namespace cyy::cxx_lib::video::ffmpeg {

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

  void reader::close() { pimpl->close(); }

  std::optional<AVCodecParameters *> reader::get_codec_parameters() {
    return pimpl->get_codec_parameters();
  }

  std::pair<int, std::shared_ptr<AVPacket>> reader::next_packet() {
    return pimpl->next_packet();
  }

} // namespace cyy::cxx_lib::video::ffmpeg
