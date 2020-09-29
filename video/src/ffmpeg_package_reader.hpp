/*!
 * \file ffmpeg_package_reader.h
 *
 * \brief 封装ffmpeg对视频流的讀操作
 * \author Yue Wu,cyy
 */
#pragma once

#include "frame.hpp"
#include "reader.hpp"
#include <memory>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
namespace cyy::cxx_lib::video::ffmpeg {

  template <bool decode_frame> class reader_impl;

  //! \brief 封装ffmpeg对视频流的讀操作
  class package_reader final {
  public:
    package_reader();

    ~package_reader();

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url);

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close();

    //! \brief 获取视频帧率
    //! \note 如果无法获取视频帧率，則返回空
    std::optional<std::array<size_t, 2>> get_frame_rate();

    //! 設置播放的幀率，用於控制next_frame的速度
    void set_play_frame_rate(const std::array<size_t, 2> &frame_rate);

    std::optional<AVCodecParameters *> get_codec_parameters();

    //! \brief 获取下一package
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容
    std::pair<int, std::shared_ptr<AVPacket>> next_packet();

  private:
    std::unique_ptr<reader_impl<false>> pimpl;
  };
} // namespace cyy::cxx_lib::video::ffmpeg
