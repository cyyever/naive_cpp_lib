/*!
 * \file reader.cpp
 *
 * \brief 封装ffmpeg对视频流的操作
 * \author Yue Wu,cyy
 */

#include "ffmpeg_audio_reader.hpp"
#include "ffmpeg_audio_reader_impl.hpp"

namespace deepir::audio::ffmpeg {

  reader::reader() : pimpl{std::make_unique<reader_impl>()} {}
  reader::~reader() = default;
  bool reader::open(const std::string &url) { return pimpl->open(url); }

  void reader::close() { pimpl->close(); }

  std::optional<std::chrono::milliseconds> reader::get_duration() {
    return pimpl->get_duration();
  }
} // namespace deepir::audio::ffmpeg
