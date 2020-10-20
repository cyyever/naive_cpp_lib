/*!
 * \file queue_scheduler.hpp
 *
 * \brief scheduler based on queue
 * \author cyy
 * \date 2018-04-18
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "base_processor.hpp"
#include "base_scheduler.hpp"
#include "data_structure/thread_safe_container.hpp"

namespace cyy::cxx_lib::task {

  //! \brief 基于队列的任务调度器
  class queue_scheduler : public base_scheduler {
  public:
    using processor_factory = std::function<std::unique_ptr<base_processor >()>;

  public:
    queue_scheduler() = default;
    queue_scheduler(const queue_scheduler &) = delete;
    queue_scheduler &operator=(const queue_scheduler &) = delete;

    explicit queue_scheduler(const std::vector<processor_factory> &makers) {
      this->replace_processor(makers);
    }

    void replace_processor(const std::vector<processor_factory> &makers) {
      std::unique_lock<std::mutex> lk(processor_mutex);
      processors = make_processors(makers);
    }

    void add_processor(const std::vector<processor_factory> &makers) {
      std::unique_lock<std::mutex> lk(processor_mutex);

      for (auto &processor : make_processors(makers)) {
        processors.emplace_back(std::move(processor));
      }
      return;
    }

    void foreach_processor(
        const std::function<bool(const base_processor *)> &call_back) {
      std::unique_lock<std::mutex> lk(processor_mutex);
      for (const auto &processor : processors) {
        if (call_back(processor.get()))
          break;
      }
    }

    //! \brief 調度任务
    //! \param timeout 任務處理超时时间
    bool schedule(const std::shared_ptr<base_task> &task,
                  const std::chrono::milliseconds &timeout) override {
      queue.push_back(task);
      return task->wait_done(timeout);
    }

    ~queue_scheduler() override {
      //我们必须在这边明确地把processors清理掉，这样做是为了处理完任务队列内积压的任务，避免内存泄露
      //在这边不能依赖processors的析构函数，因为这样的话queue和processors的析构函数的顺序依赖于成员声明的位置，很容易在重构时跪掉
      processors.clear();
    }

  private:
    using processor_list_type = std::vector<std::unique_ptr<base_processor>>;

    processor_list_type
    make_processors(const std::vector<processor_factory> &makers) {
      processor_list_type new_processors;
      //先构造新的processor
      for (auto maker : makers) {
        auto tmp = maker();
        tmp->set_get_task_func(
            [this](const std::chrono::milliseconds &timeout) {
              return queue.pop_front(timeout);
            }

        );
        new_processors.emplace_back(std::move(tmp));
      }

      for (auto &processor : new_processors) {
        processor->start();
      }
      return new_processors;
    }

  private:
    thread_safe_linear_container<std::vector<std::shared_ptr<base_task>>> queue;

    processor_list_type processors;
    std::mutex processor_mutex;
  }; // class queue_scheduler
} // namespace cyy::cxx_lib::task
