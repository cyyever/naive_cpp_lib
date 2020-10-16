/*!
 * \file base_processor.hpp
 *
 * \brief 任务处理器基类
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "base_task.hpp"
#include "util/runnable.hpp"

namespace cyy::cxx_lib::task {

  //! \brief 任務處理器
  class base_processor : public cyy::cxx_lib::runnable {
  public:
    base_processor() {}

    base_processor(const base_processor &) = delete;
    base_processor &operator=(const base_processor &) = delete;

    ~base_processor() override = default;

    virtual bool can_use_gpu() const = 0;

    void set_gpu_no(int gpu_no_) {
      if (gpu_no != -1) {
        throw std::runtime_error(std::string("processor has gpu no ") +
                                 std::to_string(gpu_no));
      }
      gpu_no = gpu_no_;
    }

    int get_gpu_no() const { return gpu_no; }

    void set_task_batch_size(size_t task_batch_size_) {
      task_batch_size = task_batch_size_;
    }

    void set_task_batch_timeout(
        const std::chrono::milliseconds &task_batch_timeout_) {
      task_batch_timeout = task_batch_timeout_;
    }

    void set_get_task_func(
        const std::function<std::optional<std::shared_ptr<base_task>>(
            const std::chrono::milliseconds &)> &get_task_func_) {
      get_task_func = get_task_func_;
    }

  protected:
    //! \brief
    //! 初始化任务处理逻辑所需的线程环境，有些第三方库需要在这边执行对应的初始化
    virtual void init_thread_context();
    //! \brief
    //! 清理任务处理逻辑所需的线程环境，有些第三方库需要在这边执行对应的清理
    virtual void deinit_thread_context() {}

  private:
    //! \brief 处理器线程主循环
    void run() override;

    //! \brief 让子类实现具体的任务处理逻辑
    virtual void
    process_tasks(std::vector<std::shared_ptr<base_task>> &tasks) = 0;

  private:
    std::function<std::optional<std::shared_ptr<base_task>>(
        const std::chrono::milliseconds &)>
        get_task_func;
    size_t task_batch_size{1};
    std::chrono::milliseconds task_batch_timeout{1000};

  protected:
    int gpu_no{-1};

  private:
    static inline std::mutex thd_mutex;

  }; // class base_processor
} // namespace cyy::cxx_lib::task
