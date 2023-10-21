/*!
 * \file runnable.hpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#ifndef __cpp_lib_jthread
#include "jthread.hpp"
#endif

namespace cyy::naive_lib {
  //! \brief runnable is a simple wrapper to std::jthread
  class runnable {
  public:
    runnable() = default;

    runnable(const runnable &) = delete;
    runnable &operator=(const runnable &) = delete;

    runnable(runnable &&) noexcept = delete;
    runnable &operator=(runnable &&) noexcept = delete;

    //! \note 子類Destructor必須明確調用stop
    virtual ~runnable() = default;
    void start(std::string_view name = "");

    template <typename WakeUpType = std::function<void()>>
    void stop(WakeUpType wakeup = []() {}) {
      while (true) {
        std::unique_lock lock(sync_mutex);
        if (!thd.joinable()) {
          break;
        }
        if (!stop_token_opt.has_value()) {
          lock.unlock();
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          continue;
        }

        std::stop_callback cb(*stop_token_opt, wakeup);
        thd.request_stop();
        thd.join();
        stop_token_opt.reset();
        thd = std::jthread();
        break;
      }
      stop_cv.notify_all();
    }
    template <typename Rep, typename Period>
    bool wait_stop(const std::chrono::duration<Rep, Period> &rel_time) {
      std::unique_lock lock(sync_mutex);
      if (!thd.joinable()) {
        return true;
      }
      return stop_cv.wait_for(lock, rel_time) == std::cv_status::no_timeout;
    }

    void wait_stop() {
      std::unique_lock lock(sync_mutex);
      if (!thd.joinable()) {
        return;
      }
      stop_cv.wait(lock);
    }

  protected:
    bool needs_stop() { return stop_token_opt->stop_requested(); }
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run(const std::stop_token &st) = 0;

    std::jthread thd;
    std::optional<std::stop_token> stop_token_opt;

  protected:
    std::mutex sync_mutex;
    std::condition_variable_any stop_cv;
  };
} // namespace cyy::naive_lib
