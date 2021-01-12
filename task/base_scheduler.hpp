/*!
 * \file base_scheduler.hpp
 *
 * \brief 任务调度器
 * \author cyy
 * \date 2016-11-09
 */
#pragma once

#include <chrono>
#include <memory>

#include "base_task.hpp"

namespace cyy::naive_lib::task{

class base_scheduler {
public:
  base_scheduler() = default;

  base_scheduler(const base_scheduler &) = delete;

  base_scheduler &operator=(const base_scheduler &) = delete;

  base_scheduler(base_scheduler &&) = default;

  base_scheduler &operator=(base_scheduler &&) = default;
  virtual ~base_scheduler() = default;

  //! \brief 調度任务
  //! \param task 任務
  //! \param timeout 任務處理超时时间
  virtual bool schedule(const std::shared_ptr<base_task> &task,
                        const std::chrono::milliseconds &timeout) = 0;

}; // class base_scheduler
} // namespace cyy::naive_lib::task
