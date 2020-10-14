/*!
 * \file queue_scheduler_test.cpp
 *
 * \author cyy
 * \date 2018-04-18
 */
#include <functional>
#include <iostream>

#include <doctest/doctest.h>

#include "../src/queue_scheduler.hpp"

class succ_processor : public cyy::cxx_lib::task::base_processor {
public:
  ~succ_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::cxx_lib::task::base_task>>
                         &tasks) override {
    for (auto &task : tasks) {
      task->finish_process(true);
    }
  }
  bool can_use_gpu() const override { return false; }
};

class fail_processor : public cyy::cxx_lib::task::base_processor {
public:
  ~fail_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::cxx_lib::task::base_task>>
                         &tasks) override {
    for (auto &task : tasks) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(50ms);
      task->finish_process(false);
    }
  }
  bool can_use_gpu() const override { return false; }
};

TEST_CASE("queue scheduler") {
  SUBCASE("finish task processing") {
    using namespace std::chrono_literals;
    cyy::cxx_lib::task::queue_scheduler scheduler(
        {[]() { return new succ_processor(); }});

    auto task_ptr = std::make_shared<cyy::cxx_lib::task::base_task>();
    scheduler.schedule(task_ptr, 100ms);
    CHECK(task_ptr->has_finished());
  }

  SUBCASE("fail to process task") {
    using namespace std::chrono_literals;
    cyy::cxx_lib::task::queue_scheduler scheduler(
        {[]() { return new fail_processor(); }});

    auto task_ptr = std::make_shared<cyy::cxx_lib::task::base_task>();
    scheduler.schedule(task_ptr, 100ms);
    CHECK(task_ptr->has_failed());
  }

  SUBCASE("process timeout") {
    using namespace std::chrono_literals;
    cyy::cxx_lib::task::queue_scheduler scheduler(
        {[]() { return new fail_processor(); }});

    auto task_ptr = std::make_shared<cyy::cxx_lib::task::base_task>();
    scheduler.schedule(task_ptr, 10ms);
    CHECK(task_ptr->has_expired());
  }
}
