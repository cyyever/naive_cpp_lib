/*!
 * \file ffmpeg_base.cpp
 *
 * \brief 封装ffmpeg庫相關函數
 * \author Yue Wu,cyy
 */

#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
}

#include "ffmpeg_base.hpp"
#include "log/log.hpp"

namespace cyy::naive_lib::video {
  //! \brief 初始化ffmpeg库
  ffmpeg_base::ffmpeg_base() {

    av_register_all();
    avdevice_register_all();
    avformat_network_init();

#ifdef NDEBUG
    av_log_set_level(AV_LOG_ERROR);
#else
    av_log_set_level(AV_LOG_VERBOSE);
#endif
  }

  std::string ffmpeg_base::errno_to_str(int err) {
    char err_buf[100]{};
    av_strerror(err, err_buf, sizeof(err_buf) - 1);
    return err_buf;
  }
} // namespace cyy::naive_lib::video
