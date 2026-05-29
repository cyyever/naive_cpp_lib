/*!
 * \file base_task_test.cpp
 *
 * \author cyy
 */

#include <chrono>
#include <thread>

#include <doctest/doctest.h>

#include "../base_task.hpp"

using namespace std::chrono_literals;
namespace task_ns = cyy::naive_lib::task;

TEST_CASE("base_task") {
  SUBCASE("void task completes") {
    task_ns::task<> task;
    CHECK(task.can_process());
    CHECK_FALSE(task.is_invalid());

    task.result_promise.set_value();
    CHECK(task.wait_done(100ms));
    CHECK_FALSE(task.can_process());
    // wait_done is idempotent for a finished task
    CHECK(task.wait_done(0ms));
  }

  SUBCASE("task_with_result returns and caches its value") {
    task_ns::task_with_result<int> task;
    task.result_promise.set_value(42);

    CHECK_EQ(task.get_result(100ms).value(), 42);
    // the cached result stays available without re-waiting
    CHECK_EQ(task.get_result(0ms).value(), 42);
  }

  SUBCASE("wait_done times out when the result is not ready") {
    task_ns::task_with_result<int> task;
    CHECK_FALSE(task.wait_done(1ms));
    CHECK_FALSE(task.get_result(1ms).has_value());
    // a timed-out task can still be processed later
    CHECK(task.can_process());
  }

  SUBCASE("mark_invalid stops a task from being processed") {
    task_ns::task<> task;
    task.mark_invalid();

    CHECK(task.is_invalid());
    CHECK_FALSE(task.can_process());
    // an invalidated task is not "done"
    CHECK_FALSE(task.wait_done(1ms));
  }

  SUBCASE("mark_invalid does not clobber an already processed task") {
    task_ns::task_with_result<int> task;
    task.result_promise.set_value(7);
    CHECK(task.wait_done(100ms));

    task.mark_invalid(); // no-op: the task is already processed
    CHECK_FALSE(task.is_invalid());
    CHECK_FALSE(task.can_process());
    CHECK_EQ(task.get_result(0ms).value(), 7);
  }

  SUBCASE("result delivered from another thread") {
    task_ns::task_with_result<int> task;
    std::thread producer([&task] {
      std::this_thread::sleep_for(20ms);
      task.result_promise.set_value(123);
    });

    CHECK(task.wait_done(1s));
    CHECK_EQ(task.get_result(0ms).value(), 123);
    producer.join();
  }
}
