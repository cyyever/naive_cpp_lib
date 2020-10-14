/*!
 * \file base_processor.cpp
 *
 * \brief 任务处理器基类
 * \author cyy
 * \date 2016-11-09
 */

#include <chrono>
#include "hardware/hardware.hpp"

#include "base_processor.hpp"
#include "base_task.hpp"

namespace cyy::cxx_lib::internal_task {

//! \brief 绑定在特定的cpu上，如果这个行为不适合特定的算法，子类可以不调用该函数
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
    std::lock_guard<std::mutex> lock(thd_mutex);
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

    for (size_t i = 0; i < tasks.size();) {
      tasks[i]->lock();
      if (tasks[i]->has_expired()) {
        tasks[i]->unlock();
        std::swap(tasks[i], tasks.back());
        tasks.pop_back();
      } else {
        i++;
      }
    }

    process_tasks(tasks);

    for (auto &task : tasks) {
      task->unlock();
    }
  }

  {
    std::lock_guard<std::mutex> lock(thd_mutex);
    deinit_thread_context();
  }
}

std::mutex base_processor::thd_mutex;
} // namespace cyy::cxx_lib::internal_task
