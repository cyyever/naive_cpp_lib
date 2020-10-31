/*!
 * \file queue_scheduler_test.cpp
 *
 * \author cyy
 * \date 2018-04-18
 */

#include <doctest/doctest.h>

#include "../src/neural_network_task.hpp"
#include "../src/queue_scheduler.hpp"

class succ_processor : public cyy::naive_lib::task::base_processor {
public:
  ~succ_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::naive_lib::task::base_task>>
                         &tasks) override {
    for (auto &task : tasks) {
      std::dynamic_pointer_cast<
          cyy::naive_lib::task::neural_network_task<int, int>>(task)
          ->result_promise.set_value(1);
    }
  }
  bool can_use_gpu() const override { return false; }
};

class timeout_processor : public cyy::naive_lib::task::base_processor {
public:
  ~timeout_processor() override { stop(); }
  void process_tasks(std::vector<std::shared_ptr<cyy::naive_lib::task::base_task>>
                         &tasks) override {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for (auto &task : tasks) {
      std::dynamic_pointer_cast<
          cyy::naive_lib::task::neural_network_task<int, int>>(task)
          ->result_promise.set_value(1);
    }
  }
  bool can_use_gpu() const override { return false; }
};

TEST_CASE("queue scheduler") {
  SUBCASE("finish task processing") {
    using namespace std::chrono_literals;
    cyy::naive_lib::task::queue_scheduler scheduler(
        {[]() { return std::make_unique<succ_processor>(); }});

    auto task_ptr =
        std::make_shared<cyy::naive_lib::task::neural_network_task<int, int>>(1);
    auto res = scheduler.schedule(task_ptr, 100ms);
    CHECK(res);
    CHECK_EQ(task_ptr->get_result().value(), 1);
  }

  SUBCASE("process timeout") {
    using namespace std::chrono_literals;
    cyy::naive_lib::task::queue_scheduler scheduler(
        {[]() { return std::make_unique<timeout_processor>(); }});

    auto task_ptr =
        std::make_shared<cyy::naive_lib::task::neural_network_task<int, int>>(1);
    auto res = scheduler.schedule(task_ptr, 10ms);
    CHECK(!res);
  }
}
