/*!
 * \file ffmpeg_writer.hpp
 *
 * \brief 封装ffmpeg对视频流的讀操作
 * \author Yue Wu,cyy
 */
#pragma once

#include <optional>
#include <memory>

#include "frame.hpp"
#include "writer.hpp"

namespace cyy::naive_lib::video {

  class ffmpeg_writer_impl;
  //! \brief 封装ffmpeg对视频流的讀操作
  class ffmpeg_writer final : public writer {
  public:
    ffmpeg_writer();

    ~ffmpeg_writer() override;

    //! \brief 打开视频
    //! \param url 视频地址，如果是本地文件，使用file://协议
    //! \note 先关闭之前打开的视频再打开此url对应的视频
    bool open(const std::string &url, const std::string &format_name,
              int video_width, int video_height,std::optional<std::pair<int,int>> frame_rate={}) override;

    //! \brief 寫入一幀
    bool write_frame(const cv::Mat &frame_mat) override;

    //! \brief 关闭已经打开的视频，如果之前没调用过open，调用该函数无效果
    void close() override;

    const std::string &get_url() const override;

  private:
    std::unique_ptr<ffmpeg_writer_impl> pimpl;
  };
} // namespace cyy::naive_lib::video
