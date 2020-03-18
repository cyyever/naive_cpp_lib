/*!
 * \file runnable.hpp
 *
 * \brief 封裝線程操作
 * \author cyy
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace cyy::cxx_lib{
  //! \brief runnable 封裝線程啓動和關閉的同步控制
  class runnable {
  public:
    void start();

    template <typename WakeUpType = std::function<void()>>
    void stop(WakeUpType wakeup = []() {}) {
      std::lock_guard<std::mutex> lock(sync_mutex);
      if (status == sync_status::running) {
        status = sync_status::wait_stop;
        wakeup();
      }
      if (thd.joinable()) {
        thd.join();
      }
      status = sync_status::no_thread;
    }

    //! \note 子類Destructor必須明確調用stop
    virtual ~runnable() = default;

    void set_name(const std::string &name_) {
      std::lock_guard<std::mutex> lock(sync_mutex);
      name = name_;
      // glibc 限制名字長度
      name.resize(15);
    }
    bool needs_stop() const { return status == sync_status::wait_stop; }

  protected:
    std::function<void(const std::exception &e)> exception_callback;

  private:
    virtual void run() = 0;

  private:
    enum class sync_status { no_thread = 0, running, wait_stop, wait_join };
    std::atomic<sync_status> status{sync_status::no_thread};
    std::thread thd;
    std::string name;
    std::mutex sync_mutex;
  };
} // namespace deepir
