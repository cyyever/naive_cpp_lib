/*!
 * \file hardware_test.cpp
 *
 * \brief 测试硬件相关函数
 * \author cyy
 */

#include <vector>

#include <doctest/doctest.h>

#include "hardware/hardware.hpp"

TEST_CASE("cpu_num") {
  CHECK(cyy::naive_lib::hardware::cpu_num() > 0);
  auto cpu_no = cyy::naive_lib::hardware::round_robin_allocator::next_cpu_no();
  CHECK(cpu_no < cyy::naive_lib::hardware::cpu_num());
}

#if defined(__linux__)
TEST_CASE("memory_size") { CHECK(cyy::naive_lib::hardware::memory_size() > 0); }


#endif
