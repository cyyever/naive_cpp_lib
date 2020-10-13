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
  CHECK(cyy::cxx_lib::hardware::cpu_num() > 0);
  auto cpu_no = cyy::cxx_lib::hardware::round_robin_allocator::next_cpu_no();
  CHECK(cpu_no < cyy::cxx_lib::hardware::cpu_num());
}

#if defined(__linux__)
TEST_CASE("memory_size") { CHECK(cyy::cxx_lib::hardware::memory_size() > 0); }

TEST_CASE("ip and mac") {
  auto my_ipv4_addresses = cyy::cxx_lib::hardware::ipv4_address();
  SUBCASE("ipv4_address") { CHECK(!my_ipv4_addresses.empty()); }
  SUBCASE("memory_size") { CHECK(cyy::cxx_lib::hardware::memory_size() > 0); }

  SUBCASE("get_mac_address_by_ipv4") {
    bool has_mac = false;
    for (const auto &addr : my_ipv4_addresses) {
      if (!cyy::cxx_lib::hardware::get_mac_address_by_ipv4(addr)) {
        has_mac = true;
      }
    }
    CHECK(has_mac);
  }
}

#endif

#ifdef _WIN32
TEST_CASE("UUID") { CHECK(!cyy::cxx_lib::hardware::UUID().empty()); }

TEST_CASE("disk_serial_number") {
  CHECK(!cyy::cxx_lib::hardware::disk_serial_number().empty());
}
#endif
