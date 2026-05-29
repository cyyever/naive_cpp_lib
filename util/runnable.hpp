/*!
 * \file runnable.hpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#pragma once

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string_view>
#include <thread>

namespace cyy::naive_lib {
  //! \brief runnable is a simple wrapper to std::jthread
  class runnable {
  public:
    runnable() noexcept = default;

    runnable(const runnable &) = delete;
    runnable &operator=(const runnable &) = delete;

    runnable(runnable &&) noexcept = delete;
    runnable &operator=(runnable &&) noexcept = delete;

    //! \note 子類Destructor必須明確調用stop
    virtual ~runnable() = default;
    void start(std::string_view name = "");

    template <typename WakeUpType = std::function<void()>>
    void stop(const WakeUpType &wakeup = []() {}) {
      std::lock_guard const lock(sync_mutex);
      if (thd.joinable()) {
        // Wake a blocked run() (via wakeup), ask it to stop, then join.
        std::stop_callback const cb(stop_source.get_token(), wakeup);
        stop_source.request_stop();
        thd.join();
        thd = std::jthread();
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
    bool needs_stop() noexcept { return stop_source.stop_requested(); }
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run(const std::stop_token &st) = 0;

    std::jthread thd;
    std::stop_source stop_source;

  protected:
    std::mutex sync_mutex;
    std::condition_variable_any stop_cv;
  };
} // namespace cyy::naive_lib
