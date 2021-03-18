/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */
#pragma once
#include "ffmpeg_converter.hpp"
#include "ffmpeg_video_reader_impl.hpp"
#include "ffmpeg_video_writer_impl.hpp"

namespace cyy::naive_lib::video {

  //! \brief 封装ffmpeg对视频流的讀操作
  class ffmpeg_converter_impl : public ffmpeg_reader_impl<false>,
                                ffmpeg_writer_impl {
  public:
    using ffmpeg_reader_impl = ffmpeg_reader_impl<false>;
    ffmpeg_converter_impl(const std::string &in_url_,
                          const std::string &out_url_)
        : in_url(in_url_), out_url(out_url_) {}

    ~ffmpeg_converter_impl() override = default;

    //! \brief 轉換視頻
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    int convert() {
      if (ffmpeg_reader_impl::has_open()) {
        if (!ffmpeg_reader_impl::open(in_url)) {
          LOG_ERROR("open url failed:{}", in_url);
          return -1;
        }
      }

      if (ffmpeg_writer_impl::has_open()) {
        if (!ffmpeg_writer_impl::open(out_url, "flv",
                                      ffmpeg_reader_impl::get_video_width(),
                                      ffmpeg_reader_impl::get_video_height())) {
          LOG_ERROR("open url failed:{}", out_url);
          return -1;
        }
      }

      while (true) {
        auto [res, pkg] = ffmpeg_reader_impl::next_packet();
        if (res <= 0) {
          return res;
        }

        if (ffmpeg_writer_impl::write_package(*pkg) != 0) {
          return -1;
        }
      }
      return 0;
    }

  private:
    std::string in_url;
    std::string out_url;
  };
} // namespace cyy::naive_lib::video
