/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */

#include "ffmpeg_converter.hpp"
#include "ffmpeg_converter_impl.hpp"

namespace cyy::cxx_lib::video::ffmpeg {
  converter::converter(const std::string &in_url, const std::string &out_url)
      : pimpl{std::make_unique<impl>(in_url, out_url)} {}
  converter::~converter() = default;
  int converter::convert() { return pimpl->convert(); }
} // namespace cyy::cxx_lib::video::ffmpeg
