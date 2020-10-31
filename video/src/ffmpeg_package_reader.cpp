/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_package_reader.hpp"

#include "ffmpeg_reader_impl.hpp"

namespace cyy::naive_lib::video::ffmpeg {

  package_reader::package_reader()
      : pimpl{std::make_unique<reader_impl<false>>()} {}
  package_reader::~package_reader() = default;
  bool package_reader::open(const std::string &url) { return pimpl->open(url); }

  std::optional<std::array<size_t, 2>> package_reader::get_frame_rate() {
    return pimpl->get_frame_rate();
  }

  void package_reader::close() { pimpl->close(); }

  std::optional<AVCodecParameters *> package_reader::get_codec_parameters() {
    return pimpl->get_codec_parameters();
  }

  void
  package_reader::set_play_frame_rate(const std::array<size_t, 2> &frame_rate) {
    return pimpl->set_play_frame_rate(frame_rate);
  }
  std::pair<int, std::shared_ptr<AVPacket>> package_reader::next_packet() {
    return pimpl->next_packet();
  }

} // namespace cyy::naive_lib::video::ffmpeg
