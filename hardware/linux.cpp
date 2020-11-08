/*!
 * \file hardware_linux.cpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#if defined(__linux__)
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
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

  std::set<std::string> mac_address() {
    struct ifreq ifr {};
    struct ifconf ifc {};
    char buf[1024]{};

    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) {
      auto socket_errno = errno;
      throw std::runtime_error(
          std::string("create UDP socket failed: ") +
          cyy::naive_lib::util::errno_to_str(socket_errno));
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(fd, SIOCGIFCONF, &ifc) != 0) {
      auto socket_errno = errno;
      close(fd);
      throw std::runtime_error(
          std::string("ioctl failed: ") +
          cyy::naive_lib::util::errno_to_str(socket_errno));
    }

    struct ifreq *it = ifc.ifc_req;
    const struct ifreq *const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    std::set<std::string> res;

    for (; it != end; ++it) {
      strncpy(ifr.ifr_name, it->ifr_name, sizeof(ifr.ifr_name) - 1);
      if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0) {
        auto socket_errno = errno;
        close(fd);
        throw std::runtime_error(
            std::string("ioctl failed: ") +
            cyy::naive_lib::util::errno_to_str(socket_errno));
      }
      if (ifr.ifr_flags & IFF_LOOPBACK) {
        continue;
      }
      if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
        auto socket_errno = errno;
        close(fd);
        throw std::runtime_error(
            std::string("ioctl failed: ") +
            cyy::naive_lib::util::errno_to_str(socket_errno));
      }

      const char *hex = "0123456789ABCDEF";
      std::string mac_addr;

      for (size_t i = 0; i < 6; i++) {
        uint8_t byte = ifr.ifr_hwaddr.sa_data[i];
        mac_addr.push_back(hex[byte >> 4]);
        mac_addr.push_back(hex[byte & 0x0F]);
      }
      res.insert(mac_addr);
    }
    close(fd);
    return res;
  }

  std::set<std::string> ipv4_address() {

    std::set<std::string> res;

    struct ifaddrs *ifas = nullptr;

    if (getifaddrs(&ifas) != 0) {
      auto tmp_errno = errno;
      throw std::runtime_error(std::string("getifaddrs failed: ") +
                               cyy::naive_lib::util::errno_to_str(tmp_errno));
    }

    if (!ifas) {
      return res;
    }

    std::unique_ptr<ifaddrs, decltype(freeifaddrs) *> ifaddrs_RAII(ifas,
                                                                   freeifaddrs);

    for (auto ifa = ifas; ifa != nullptr; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr) {
        continue;
      }
      if (ifa->ifa_addr->sa_family != AF_INET) { // check it is IP4
        continue;
      }
      if (!(ifa->ifa_flags & IFF_UP)) {
        continue;
      }
      if (!(ifa->ifa_flags & IFF_RUNNING)) {
        continue;
      }
      if (ifa->ifa_flags & IFF_LOOPBACK) {
        continue;
      }
      // is a valid IP4 Address
      char buf[INET_ADDRSTRLEN + 1]{};
      if (!inet_ntop(AF_INET,
                     &(reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr))
                          ->sin_addr,
                     buf, INET_ADDRSTRLEN + 1)) {
        auto tmp_errno = errno;
        throw std::runtime_error(std::string("inet_ntop failed: ") +
                                 cyy::naive_lib::util::errno_to_str(tmp_errno));
      }
      res.insert(buf);
    }

    return res;
  }

  std::optional<std::string>
  get_mac_address_by_ipv4(const std::string &ipv4_addr) {
    std::ifstream arp_file("/proc/net/arp");

    char buf[1024];
    while (arp_file) {
      arp_file.getline(buf, sizeof(buf));
      if (!arp_file) {
        return {};
      }

      if (!strstr(buf, ipv4_addr.c_str())) {
        continue;
      }

      std::regex mac_regex("[a-z0-9]{2}:[a-z0-9]{2}:[a-z0-9]{2}:[a-z0-9]{2}:[a-"
                           "z0-9]{2}:[a-z0-9]{2}",
                           std::regex_constants::ECMAScript |
                               std::regex_constants::icase);

      std::cmatch match;
      if (std::regex_search(buf, match, mac_regex)) {
        auto mac_address = match[0].str();
        mac_address.erase(
            std::remove(mac_address.begin(), mac_address.end(), ':'),
            mac_address.end());
        return {mac_address};
      }
    }
    return {};
  }

} // namespace cyy::naive_lib::hardware

#endif
