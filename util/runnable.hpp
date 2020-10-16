/*!
 * \file runnable.hpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

#include <condition_variable>

namespace cyy::cxx_lib {
  //! \brief runnable 封裝線程啓動和關閉的同步控制
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
      std::unique_lock lock(sync_mutex);
      if (!thd.joinable()) {
        return;
      }
      std::stop_callback cb(thd.get_stop_token(), wakeup);
      thd.request_stop();
      lock.unlock();
      thd.join();
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
    bool needs_stop() {
      std::unique_lock lock(sync_mutex);
      if (!thd.joinable()) {
        return true;
      }
      return thd.get_stop_token().stop_requested();
    }

  protected:
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run() = 0;

  private:
    std::jthread thd;

  protected:
    std::recursive_mutex sync_mutex;
    std::condition_variable_any stop_cv;
  };
} // namespace cyy::cxx_lib
