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

    //! \note 子類仍應在自己的析構函數中調用stop（以便傳入喚醒回調）；
    //! 這裡的stop()只是兜底，確保工作線程在對象銷毀前結束
    virtual ~runnable() { stop(); }
    void start(std::string_view name = "");

    template <typename WakeUpType = std::function<void()>>
    void stop(const WakeUpType &wakeup = []() {}) {
      std::unique_lock lock(sync_mutex);
      if (thd.joinable()) {
        // Wake a blocked run() (via wakeup) and ask it to stop.
        std::stop_callback const cb(stop_source.get_token(), wakeup);
        stop_source.request_stop();
        // Drop the lock so the worker can publish `finished` and notify before
        // we join; the worker never touches `thd`, so joining unlocked is
        // race-free under the single-owner contract.
        lock.unlock();
        thd.join();
        lock.lock();
        thd = std::jthread();
      }
    }
    template <typename Rep, typename Period>
    bool wait_stop(const std::chrono::duration<Rep, Period> &rel_time) {
      std::unique_lock lock(sync_mutex);
      return stop_cv.wait_for(lock, rel_time, [this] { return finished; });
    }

    void wait_stop() {
      std::unique_lock lock(sync_mutex);
      stop_cv.wait(lock, [this] { return finished; });
    }

  protected:
    bool needs_stop() noexcept { return stop_source.stop_requested(); }
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run(const std::stop_token &st) = 0;

    std::jthread thd;
    std::stop_source stop_source;
    //! \brief run()是否已結束（受sync_mutex保護），用於wait_stop的條件判斷
    bool finished{true};

  protected:
    std::mutex sync_mutex;
    std::condition_variable_any stop_cv;
  };
} // namespace cyy::naive_lib
