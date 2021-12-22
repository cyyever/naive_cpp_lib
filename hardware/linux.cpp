/*!
 * \file hardware_linux.cpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#if defined(__linux__)
#include <cerrno>
#include <fstream>
#include <ifaddrs.h>
#include <pthread.h>
#include <regex>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "hardware.hpp"
#include "util/error.hpp"

namespace cyy::naive_lib::hardware {

  static void alloc_cpu(pthread_t pt, size_t cpu_no) {
    cpu_set_t cpuset{};
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_no, &cpuset);
    pthread_setaffinity_np(pt, sizeof(cpu_set_t), &cpuset);
  }
  void alloc_cpu(std::thread &thd, size_t cpu_no) {
    alloc_cpu(thd.native_handle(), cpu_no);
  }

  size_t memory_size() {
    auto physPages = sysconf(_SC_PHYS_PAGES);
    auto pageSize = sysconf(_SC_PAGESIZE);
    if (physPages <= 0 || pageSize <= 0) {
      throw std::runtime_error("sysconf failed");
    }
    return static_cast<size_t>(physPages) * static_cast<size_t>(pageSize);
  }




} // namespace cyy::naive_lib::hardware

#endif
