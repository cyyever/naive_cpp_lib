/*!
 * \file monitor_test.cpp
 *
 * \brief 测试進程監控
 */
#include <chrono>
#include <csignal>
#include <functional>
#include <thread>

#include <doctest/doctest.h>
#include <sys/types.h>

#include "process/src/process.hpp"

TEST_CASE("monitor") {

  REQUIRE(cyy::naive_lib::process::monitor::init());

  auto test_start_process = []() {
    using namespace std::chrono_literals;
    auto child_id_opt = cyy::naive_lib::process::monitor::start_process(
        {"sh",
         {"sh", "-c", "sleep 1;echo b >&${CHANNEL_FD};  sleep 5 "},
         {},
         true},
        1s, 1);
    CHECK(child_id_opt);
    CHECK(cyy::naive_lib::process::monitor::monitored_process_exist(
        child_id_opt.value()));
    return child_id_opt.value();
  };
  SUBCASE("child fail") {
    using namespace std::chrono_literals;
    auto child_id_opt = cyy::naive_lib::process::monitor::start_process(
        {"sh", {"sh", "-c", "exit 3"}}, 1s, 2);
    CHECK(child_id_opt);
    std::this_thread::sleep_for(5s);
    CHECK(!cyy::naive_lib::process::monitor::monitored_process_exist(
        child_id_opt.value()));
  }

  SUBCASE("kill child") {
    using namespace std::chrono_literals;
    auto child_id = test_start_process();
    CHECK(cyy::naive_lib::process::monitor::signal_monitored_process(child_id,
                                                                   SIGKILL));
    std::this_thread::sleep_for(2s);
    CHECK(!cyy::naive_lib::process::monitor::monitored_process_exist(child_id));
  }

  SUBCASE("write channel") {
    auto child_id = test_start_process();
    CHECK(cyy::naive_lib::process::monitor::write_monitored_process(
        child_id, gsl::not_null<const void *>("a"), 1));
  }

  SUBCASE("read channel") {
    using namespace std::chrono_literals;
    auto child_id = test_start_process();
    std::this_thread::sleep_for(5s);
    auto data_opt =
        cyy::naive_lib::process::monitor::read_monitored_process(child_id, 1);
    REQUIRE(data_opt);
    REQUIRE(data_opt.value().size() == 1);
    REQUIRE(data_opt.value()[0] == std::byte('b'));
  }
}
