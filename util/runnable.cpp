/*!
 * \file runnable.cpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#include "log/src/log.hpp"

#include "error.hpp"
#include "runnable.hpp"

namespace cyy::cxx_lib {
  void runnable::start() {
    std::lock_guard<std::mutex> lock(sync_mutex);
    if (status != sync_status::no_thread) {
      throw std::runtime_error("thread is running");
    }
    status = sync_status::running;
    try {
      thd = std::thread([this]() {
        try {
#if defined(__linux__)
          if (!name.empty()) {
            auto err = pthread_setname_np(pthread_self(), name.c_str());
            if (err != 0) {
              LOG_ERROR("pthread_setname_np failed:{}",
                        cyy::cxx_lib::util::errno_to_str(err));
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
        status = sync_status::wait_join;
      });
    } catch (const std::exception &e) {
      status = sync_status::no_thread;
      if (exception_callback) {
        exception_callback(e);
      }
      LOG_ERROR("create thread failed:{}", e.what());
    }
  }

} // namespace cyy::cxx_lib
