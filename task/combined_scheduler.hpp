/*!
 * \file combined_scheduler.hpp
 *
 * \brief combine several schedulers
 */
#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "base_processor.hpp"
#include "base_scheduler.hpp"

namespace cyy::naive_lib::task {

  //! \brief 組合數個任务调度器
  class combined_scheduler final {
  public:
    using processor_factory = std::function<std::unique_ptr<base_processor>()>;

    using task_converter_type = std::function<std::shared_ptr<base_task>(
        size_t, const std::shared_ptr<base_task> &)>;

  public:
    explicit combined_scheduler(
        std::vector<std::shared_ptr<base_scheduler>> schedulers_,
        task_converter_type task_converter_)
        : schedulers{std::move(schedulers_)},
          task_converter{std::move(task_converter_)} {}
    combined_scheduler(const combined_scheduler &) = delete;
    combined_scheduler &operator=(const combined_scheduler &) = delete;
    combined_scheduler(combined_scheduler &&) = default;

    combined_scheduler &operator=(combined_scheduler &&) = default;

    //! \brief 調度任务
    //! \param task 任務
    //! \param timeout 任務處理超时时间
    //! \return 任務
    std::shared_ptr<base_task>
    schedule(const std::shared_ptr<base_task> &task,
             const std::chrono::milliseconds &timeout) {
      auto end_tp = std::chrono::steady_clock::now() + timeout;
      std::shared_ptr<base_task> cur_task = task;
      for (size_t i = 0; i < schedulers.size(); i++) {
        if (!schedulers[i]->schedule(
                cur_task, std::chrono::duration_cast<std::chrono::milliseconds>(
                              end_tp - std::chrono::steady_clock::now()))) {
          return {};
        }
        cur_task = task_converter(i, cur_task);
      }
      return cur_task;
    }

  private:
    std::vector<std::shared_ptr<base_scheduler>> schedulers;
    task_converter_type task_converter;
  }; // class combined_scheduler
} // namespace cyy::naive_lib::task
