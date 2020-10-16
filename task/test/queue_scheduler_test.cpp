/*!
 * \file queue_scheduler_test.cpp
 *
 * \author cyy
 * \date 2018-04-18
 */

#include <doctest/doctest.h>

#include "../src/neural_network_task.hpp"
#include "../src/queue_scheduler.hpp"

class succ_processor : public cyy::cxx_lib::task::base_processor {
public:
  ~succ_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::cxx_lib::task::base_task>>
                         &tasks) override {
    for (auto &task : tasks) {
      std::dynamic_pointer_cast<
          cyy::cxx_lib::task::neural_network_task<int, int>>(task)
          ->result_promise.set_value(1);
    }
  }
  bool can_use_gpu() const override { return false; }
};

class timeout_processor : public cyy::cxx_lib::task::base_processor {
public:
  ~timeout_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::cxx_lib::task::base_task>>
                         &tasks) override {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (auto &task : tasks) {
      std::dynamic_pointer_cast<
          cyy::cxx_lib::task::neural_network_task<int, int>>(task)
          ->result_promise.set_value(1);
    }
  }
  bool can_use_gpu() const override { return false; }
};

TEST_CASE("queue scheduler") {
  SUBCASE("finish task processing") {
    using namespace std::chrono_literals;
    cyy::cxx_lib::task::queue_scheduler scheduler(
        {[]() { return new succ_processor(); }});

    auto task_ptr =
        std::make_shared<cyy::cxx_lib::task::neural_network_task<int, int>>(1);
    scheduler.schedule(task_ptr, 100ms);
    CHECK(task_ptr->has_finished());
    CHECK_EQ(task_ptr->get_result().value(),1);
  }

  SUBCASE("process timeout") {
    using namespace std::chrono_literals;
    cyy::cxx_lib::task::queue_scheduler scheduler(
        {[]() { return new timeout_processor(); }});

    auto task_ptr =
        std::make_shared<cyy::cxx_lib::task::neural_network_task<int, int>>(1);
    scheduler.schedule(task_ptr, 10ms);
    CHECK(task_ptr->has_expired());
  }
}
