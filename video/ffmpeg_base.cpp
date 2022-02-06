/*!
 * \file ffmpeg_base.cpp
 *
 * \brief 封装ffmpeg庫相關函數
 * \author Yue Wu,cyy
 */

#include <regex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
}

#include "ffmpeg_base.hpp"
#include "log/log.hpp"

namespace cyy::naive_lib::video {
  //! \brief 初始化ffmpeg库
  ffmpeg_base::ffmpeg_base() {

    /* av_register_all(); */
    avdevice_register_all();
    avformat_network_init();

#ifdef NDEBUG
    av_log_set_level(AV_LOG_ERROR);
#else
    av_log_set_level(AV_LOG_VERBOSE);
#endif
  }
  bool ffmpeg_base::open(const std::string &url_) {
    url = url_;
    this->close();

    std::regex scheme_regex("([a-z][a-z0-9+-.]+)://",
                            std::regex_constants::ECMAScript |
                                std::regex_constants::icase);

    std::smatch match;
    if (std::regex_search(url, match, scheme_regex)) {
      url_scheme = match[1].str();
      for (auto &c : url_scheme) {
        if (c >= 'A' && c <= 'Z') {
          c = c - 'A' + 'a';
        }
      }
    } else {
      url_scheme = "file";
    }
    return true;
  }

  std::string ffmpeg_base::errno_to_str(int err) {
    char err_buf[100]{};
    av_strerror(err, err_buf, sizeof(err_buf) - 1);
    return err_buf;
  }
} // namespace cyy::naive_lib::video
