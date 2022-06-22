/*!
 * \file runnable.cpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#include "runnable.hpp"

#include "error.hpp"
#include "log/log.hpp"

namespace cyy::naive_lib {
  void runnable::start(std::string name) {
    std::lock_guard lock(sync_mutex);
    if (thd.joinable()) {
      throw std::runtime_error("thread is running");
    }
    try {
      thd = std::jthread(
          [this](std::stop_token st, [[maybe_unused]] std::string name_) {
            try {
#if defined(__linux__)
              if (!name_.empty()) {
                // glibc 限制名字長度
                name_.resize(15);
                auto err = pthread_setname_np(pthread_self(), name_.c_str());
                if (err != 0) {
                  LOG_ERROR("pthread_setname_np failed:{}",
                            cyy::naive_lib::util::errno_to_str(err));
                }
              }
#endif
              run();
            } catch (const std::exception &e) {
              if (exception_callback) {
                exception_callback(e);
              }
              LOG_ERROR("catch thread exception:{}", e.what());
            }
            stop_cv.notify_all();
          },
          std::move(name));
    } catch (const std::exception &e) {
      stop_cv.notify_all();
      if (exception_callback) {
        exception_callback(e);
      }
      LOG_ERROR("create thread failed:{}", e.what());
      throw;
    }
  }

} // namespace cyy::naive_lib
