/*!
 * \file base_processor.cpp
 *
 * \brief 任务处理器基类
 * \author cyy
 * \date 2016-11-09
 */

#include "base_processor.hpp"

#include <chrono>

#include "base_task.hpp"
#include "hardware/hardware.hpp"
#include "log/log.hpp"

namespace cyy::cxx_lib::task {

  //! \brief
  //! 绑定在特定的cpu上，如果这个行为不适合特定的算法，子类可以不调用该函数
  void base_processor::init_thread_context() {
    /*
    cyy::cxx_lib::hardware::alloc_cpu(
        this_thd, cyy::cxx_lib::hardware::round_robin_allocator::next_cpu_no());
    */
  }

  void base_processor::run() {
    if (!get_task_func) {
      throw std::runtime_error("get_task_func is empty");
    }

    {
      std::lock_guard lock(thd_mutex);
      init_thread_context();
    }

    while (!needs_stop()) {
      std::vector<std::shared_ptr<base_task>> tasks;

      auto end_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now() + task_batch_timeout);

      while (tasks.size() < task_batch_size) {
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now());
        if (now_ms >= end_ms) {
          break;
        }

        auto task_opt = get_task_func(end_ms - now_ms);
        if (task_opt) {
          tasks.push_back(*task_opt);
        }
      }

      auto num =
          std::erase_if(tasks, [](auto const &p) { return p->has_expired(); });
      if (num != 0) {
        LOG_WARN("skip {} expired tasks", num);
      }
      process_tasks(tasks);
    }

    {
      std::lock_guard lock(thd_mutex);
      deinit_thread_context();
    }
  }

} // namespace cyy::cxx_lib::task
