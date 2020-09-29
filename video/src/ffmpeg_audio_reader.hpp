/*!
 * \file ffmpeg_audio_reader.h
 *
 * \brief 封装ffmpeg对sound频流的讀操作
 * \author Yue Wu,cyy
 */
#pragma once

#include <memory>

#include "audio_reader.hpp"
namespace deepir::audio::ffmpeg {

class reader_impl;
//! \brief 封装ffmpeg对视频流的讀操作
class reader final : public ::deepir::audio::reader {
public:
  reader();

  ~reader() override;

  //! \brief 打开视频
  //! \param url 视频地址，如果是本地文件，使用file://协议
  //! \note 先关闭之前打开的视频再打开此url对应的视频
  bool open(const std::string &url) override;

  //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
  void close() override;

  std::optional<std::chrono::milliseconds> get_duration() override;

private:
  std::unique_ptr<reader_impl> pimpl;
};
} // namespace deepir::audio::ffmpeg
