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

namespace cyy::cxx_lib::video::ffmpeg {

  namespace {
    //! \brief RAII类用于封装ffmpeg库初始化
    class initer final {
    public:
      initer() {
        av_register_all();
        avdevice_register_all();
        avformat_network_init();

#ifdef NDEBUG
        av_log_set_level(AV_LOG_ERROR);
#else
        av_log_set_level(AV_LOG_VERBOSE);
#endif

        if (av_lockmgr_register(lock_manager) != 0) {
          throw std::runtime_error("av_lockmgr_register failed");
        }
      }

    private:
      //! \brief ffmpeg库要求我们提供该函数来实现线程安全
      static int lock_manager(void **mutex, enum AVLockOp op) {
#ifdef _WIN32
        LOG_error("windows not support thread safty");
        return 1;
#endif
        switch (op) {
          case AV_LOCK_CREATE:
            *mutex = malloc(sizeof(pthread_mutex_t));
            if (!*mutex) {
              return 1;
            }
            return pthread_mutex_init(
                reinterpret_cast<pthread_mutex_t *>(*mutex), nullptr);
          case AV_LOCK_OBTAIN:
            return pthread_mutex_lock(
                reinterpret_cast<pthread_mutex_t *>(*mutex));
          case AV_LOCK_RELEASE:
            return pthread_mutex_unlock(
                reinterpret_cast<pthread_mutex_t *>(*mutex));
          case AV_LOCK_DESTROY:
            pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(*mutex));
            free(*mutex);
            return 0;
        }
        return 1;
      }
    };
  } // namespace

  //! \brief 初始化ffmpeg库
  void init_library() { static initer initer; }

  std::string errno_to_str(int err) {
    char err_buf[100]{};
    av_strerror(err, err_buf, sizeof(err_buf) - 1);
    return err_buf;
  }
} // namespace cyy::cxx_lib::video::ffmpeg
