/*!
 * \file runnable.cpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */

#include "error.hpp"
#include "log/log.hpp"
#include "runnable.hpp"

#include <stdexcept>
#include <string>

namespace cyy::naive_lib {
  void runnable::start(std::string_view name) {
    std::lock_guard const lock(sync_mutex);
    if (thd.joinable()) {
      throw std::runtime_error("thread is running");
    }
    // Own the stop source so the worker never has to publish the jthread's
    // stop_token back into this object across threads.
    stop_source = std::stop_source();
    finished = false;
    try {
      thd = std::jthread(
          [this]([[maybe_unused]] const std::stop_token &st,
                 [[maybe_unused]] const std::string &name_) {
            try {
              if (!name_.empty()) {
                cyy::naive_lib::log::set_thread_name(name_);
              }
              run(stop_source.get_token());
            } catch (const std::exception &e) {
              if (exception_callback) {
                exception_callback(e);
              }
              LOG_ERROR("catch thread exception:{}", e.what());
            }
            // Publish completion under the lock so wait_stop() cannot miss it.
            {
              std::lock_guard const done_lock(sync_mutex);
              finished = true;
            }
            stop_cv.notify_all();
          },
          std::string(name));
    } catch (const std::exception &e) {
      // The worker was never created, so mark completion here.
      finished = true;
      stop_cv.notify_all();
      if (exception_callback) {
        exception_callback(e);
      }
      LOG_ERROR("create thread failed:{}", e.what());
      throw;
    }
  }

} // namespace cyy::naive_lib
