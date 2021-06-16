/*!
 * \file ffmpeg_reader.h
 *
 * \brief 封装ffmpeg对视频流的讀操作
 * \author Yue Wu,cyy
 */
#pragma once

#include <functional>
#include <memory>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "frame.hpp"
#include "video_reader.hpp"
namespace cyy::naive_lib::video {
  template <bool decode_frame> class ffmpeg_reader_impl;
  //! \brief 封装ffmpeg对视频流的讀操作
  class ffmpeg_reader final : public reader {
  public:
    ffmpeg_reader();

    ~ffmpeg_reader() override;

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url) override;

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() noexcept override;

    //! \brief 获取視頻寬
    std::optional<int> get_video_width() const;
    //! \brief 获取視頻高
    std::optional<int> get_video_height() const;

    //! 設置播放的幀率，用於控制next_frame的速度
    void set_play_frame_rate(const std::array<size_t, 2> &frame_rate);

    //! \brief 获取视频帧率
    //! \note 如果无法获取视频帧率，則返回空
    std::optional<std::array<size_t, 2>> get_frame_rate() override;

    void add_named_filter(std::string name, std::function<bool(size_t)> filter);

    void remove_named_filter(std::string name);
    void keep_non_key_frames();
    void drop_non_key_frames();

    //! \brief 获取下一帧
    //! \return first>0 成功
    //	      first=0 EOF
    //	      first<0 失敗
    //	如果first<=0，返回空内容
    std::pair<int, frame> next_frame() override;

    //! \brief jump to a frame
    bool seek_frame(size_t frame_seq) override;

  private:
    std::unique_ptr<ffmpeg_reader_impl<true>> pimpl;
  };
} // namespace cyy::naive_lib::video
