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
    void start(std::string name = "");

    template <typename WakeUpType = std::function<void()>>
    void stop(WakeUpType wakeup = []() {}) {
      {
        std::lock_guard lock(sync_mutex);
        if (!thd.joinable()) {
          return;
        }
        std::stop_callback cb(st, wakeup);
        thd.request_stop();
        thd.join();
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
    bool needs_stop() { return st.stop_requested(); }

  protected:
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run() = 0;

  private:
    std::jthread thd;

  protected:
    std::stop_token st;
    std::mutex sync_mutex;
    std::condition_variable_any stop_cv;
  };
} // namespace cyy::naive_lib
