/*!
 * \file ffmpeg_converter.cpp
 *
 * \brief
 */
/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */

#include "ffmpeg_converter.hpp"

#include "ffmpeg_converter_impl.hpp"

namespace cyy::naive_lib::video {
  ffmpeg_converter::ffmpeg_converter(const std::string &in_url,
                                     const std::string &out_url)
      : pimpl{std::make_unique<ffmpeg_converter_impl>(in_url, out_url)} {}
  ffmpeg_converter::~ffmpeg_converter() = default;
  int ffmpeg_converter::convert() { return pimpl->convert(); }
} // namespace cyy::naive_lib::video
