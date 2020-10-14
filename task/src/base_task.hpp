/*!
 * \file base_task.hpp
 *
 * \brief 任务基类
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include <chrono>
#include <mutex>

#include <condition_variable>

namespace cyy::cxx_lib::task {

  //! \brief 綫程任务
  class base_task {
  public:
    base_task() = default;

    base_task(const base_task &) = delete;
    base_task &operator=(const base_task &) = delete;

    virtual ~base_task() = default;

    void lock() { task_mutex.lock(); }
    bool try_lock() { return task_mutex.try_lock(); }
    void unlock() { task_mutex.unlock(); }

    //! \brief 处理任务
    //! \param timeout 该线程等待超时时间
    //! \param callback call after change status
    virtual void process(const std::chrono::milliseconds &timeout,
                         std::function<void()> callback) {
      std::unique_lock<std::mutex> lk(task_mutex);
      if (status != task_status::unprocessed &&
          status != task_status::timed_out) {
        return;
      }
      try {
        status = task_status::processing;
        callback();
        if (task_cv.wait_for(lk, timeout) == std::cv_status::timeout) {
          status = task_status::timed_out;
          return;
        }
      } catch (...) { //视为超时
        status = task_status::timed_out;
      }
    }

    //! \brief 重置任务状态，重新开始处理任务
    void restart_process() {
      std::unique_lock<std::mutex> lk(task_mutex);
      status = task_status::unprocessed;
    }

    bool has_expired() const { return status == task_status::timed_out; }

    bool has_finished() const { return status == task_status::processed; }

    bool has_failed() const { return status == task_status::process_failed; }

    //! \brief 标识任务处理完毕
    //! \param succ 任务处理是否成功
    void finish_process(bool succ) {
      if (status == task_status::timed_out) {
        return;
      }
      auto pre_status = status;

      if (succ)
        status = task_status::processed;
      else
        status = task_status::process_failed;
      if (pre_status == task_status::processing)
        task_cv.notify_one();
      return;
    }

  private:
    //! \brief 任务状态，用于做processor和task的信息交换
    enum class task_status : uint8_t {
      unprocessed,
      processing,
      timed_out,
      process_failed,
      processed
    };

  private:
    task_status status{task_status::unprocessed};
    std::mutex task_mutex;
    std::condition_variable task_cv;
  };
} // namespace cyy::cxx_lib::task
