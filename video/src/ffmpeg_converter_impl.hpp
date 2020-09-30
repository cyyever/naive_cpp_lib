/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */

#include "ffmpeg_converter.hpp"
#include "ffmpeg_reader_impl.hpp"
#include "ffmpeg_writer_impl.hpp"

namespace cyy::cxx_lib::video::ffmpeg {

  //! \brief 封装ffmpeg对视频流的讀操作
  class converter::impl : public reader_impl<false>, writer::impl {
  public:
    using reader_impl = reader_impl<false>;
    impl(const std::string &in_url_, const std::string &out_url_)
        : in_url(in_url_), out_url(out_url_) {}

    ~impl() override = default;

    //! \brief 轉換視頻
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    int convert() {
      if (reader_impl::has_open()) {
        if (!reader_impl::open(in_url)) {
          LOG_ERROR("open url failed:{}", in_url);
          return -1;
        }
      }

      if (writer::impl::has_open()) {
        if (!writer::impl::open(out_url, "flv", reader_impl::get_video_width(),
                                reader_impl::get_video_height())) {
          LOG_ERROR("open url failed:{}", out_url);
          return -1;
        }
      }

      while (true) {
        auto [res, pkg] = reader_impl::next_packet();
        if (res <= 0) {
          return res;
        }

        if (writer::impl::write_package(*pkg) != 0) {
          return -1;
        }
      }
      return 0;
    }

  private:
    std::string in_url;
    std::string out_url;
  };
} // namespace cyy::cxx_lib::video::ffmpeg
