/*!
 * \file process_test.cpp
 *
 * \author cyy
 */

#include <vector>

#include <doctest/doctest.h>

#include "process/src/process.hpp"

TEST_CASE("spawn") {
  CHECK(!::cyy::naive_lib::process::spawn({"/", {"/"}}));

  auto spawn_result_opt = ::cyy::naive_lib::process::spawn(
      {"sh", {"sh", "-c", "echo ${CHANNEL_FD} "}, {}, true});
  CHECK(spawn_result_opt);
  CHECK(spawn_result_opt->channel_fd);
  CHECK_GE(spawn_result_opt->channel_fd.value(), 0);
  close(spawn_result_opt->channel_fd.value());
}
