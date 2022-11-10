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
              {
                while (true) {
                  if (this->sync_mutex.try_lock()) {
                    this->stop_token_opt = st;
                    this->sync_mutex.unlock();
                    break;
                  }
                  std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
              }
              if (!name_.empty()) {
                cyy::naive_lib::log::set_thread_name(name_);
              }
              run(st);
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
