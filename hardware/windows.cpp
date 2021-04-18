/*!
 * \file hardware.cpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <cassert>
#include <cerrno>
#include <comdef.h>
#include <iostream>
#include <iphlpapi.h>
#include <locale>
#include <memory>
#include <mutex>
#include <string>
#include <ws2tcpip.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

#include "hardware.hpp"
#include "log/log.hpp"
#include "util/error.hpp"
#include "util/string.hpp"

namespace cyy::naive_lib::hardware {

  std::set<std::string> mac_address() {
    DWORD dwRetVal = 0;

    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;

    ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES) * 2;

    for (size_t i = 0; i < 2; i++) {
      pAddresses =
          new IP_ADAPTER_ADDRESSES[outBufLen / sizeof(IP_ADAPTER_ADDRESSES) +
                                   1];

      ULONG flags = GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST |
                    GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER |
                    GAA_FLAG_SKIP_FRIENDLY_NAME;

      dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses,
                                      &outBufLen);

      if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        delete[] pAddresses;
        pAddresses = nullptr;
      } else {
        break;
      }
    }

    if (dwRetVal != NO_ERROR) {
      return {};
    }

    std::unique_ptr<IP_ADAPTER_ADDRESSES[]> pAddresses_RAII(pAddresses);

    std::set<std::string> res;

    for (auto pCurrAddresses = pAddresses; pCurrAddresses;
         pCurrAddresses = pCurrAddresses->Next) {
      if (pCurrAddresses->PhysicalAddressLength != 6) {
        continue;
      }

      const char *hex = "0123456789ABCDEF";
      std::string mac_addr;

      for (size_t i = 0; i < pCurrAddresses->PhysicalAddressLength; i++) {
        uint8_t byte = pCurrAddresses->PhysicalAddress[i];
        mac_addr.push_back(hex[byte >> 4]);
        mac_addr.push_back(hex[byte & 0x0F]);
      }
      res.insert(mac_addr);
    }
    return res;
  }

  std::set<std::string> ipv4_address() {
    DWORD dwRetVal = 0;

    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;

    ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES) * 2;

    for (size_t i = 0; i < 2; i++) {
      pAddresses =
          new IP_ADAPTER_ADDRESSES[outBufLen / sizeof(IP_ADAPTER_ADDRESSES) +
                                   1];

      ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                    GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;

      dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, pAddresses,
                                      &outBufLen);

      if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        delete[] pAddresses;
        pAddresses = nullptr;
      } else {
        break;
      }
    }

    if (dwRetVal != NO_ERROR) {
      return {};
    }

    std::unique_ptr<IP_ADAPTER_ADDRESSES[]> pAddresses_RAII(pAddresses);
    std::set<std::string> res;

    for (auto pCurrAddresses = pAddresses; pCurrAddresses;
         pCurrAddresses = pCurrAddresses->Next) {
      if (pCurrAddresses->PhysicalAddressLength != 6) {
        continue;
      }

      if (pCurrAddresses->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
          pCurrAddresses->IfType == IF_TYPE_TUNNEL ||
          pCurrAddresses->IfType == IF_TYPE_OTHER) {
        continue;
      }

      if (pCurrAddresses->TunnelType != TUNNEL_TYPE_NONE) {
        continue;
      }

      for (auto address = pCurrAddresses->FirstUnicastAddress;
           address != nullptr; address = address->Next) {
        auto family = address->Address.lpSockaddr->sa_family;
        if (AF_INET == family) {
          // IPv4
          SOCKADDR_IN *ipv4 =
              reinterpret_cast<SOCKADDR_IN *>(address->Address.lpSockaddr);

          char str_buffer[INET_ADDRSTRLEN + 1]{};

          InetNtop(AF_INET, &(ipv4->sin_addr), str_buffer, INET_ADDRSTRLEN);
          res.insert(str_buffer);
          break;
        }
      }
    }
    return res;
  }

  namespace {
    class COM_initer final {
    public:
      COM_initer() {
        if (CoInitializeEx(0, COINIT_MULTITHREADED) != S_OK) {
          throw std::runtime_error("CoInitializeEx failed");
        }
      }
      ~COM_initer() { CoUninitialize(); }
    };

    // std::mutex windows_hardware_mutex;
  } // namespace



  std::string get_mac_address_by_ipv4(const std::string &ipv4_addr) {

    DWORD dwAddr;
    {
      if (InetPton(AF_INET, ipv4_addr.c_str(), &dwAddr) != 1) {
        LOG_ERROR("InetPton failed:{}", WSAGetLastError());
        return "";
      }
    }

    ULONG size = 0;
    auto res = GetIpNetTable(nullptr, &size, FALSE);
    if (res == ERROR_NO_DATA) {
      return "";
    } else if (res != ERROR_INSUFFICIENT_BUFFER) {
      return "";
    }

    std::vector<char> buf(size, {});

    auto pIpNetTable = reinterpret_cast<PMIB_IPNETTABLE>(buf.data());

    res = GetIpNetTable(pIpNetTable, &size, FALSE);
    if (res == ERROR_NO_DATA) {
      return "";
    } else if (res != NO_ERROR) {
      return "";
    }

    for (DWORD i = 0; i < pIpNetTable->dwNumEntries; i++) {
      if (pIpNetTable->table[i].dwType == MIB_IPNET_TYPE_INVALID) {
        continue;
      }

      if (pIpNetTable->table[i].dwAddr != dwAddr) {
        continue;
      }

      char mac_addr[13]{};

      assert(snprintf(mac_addr, sizeof(mac_addr), "%02x%02x%02x%02x%02x%02x",
                      pIpNetTable->table[i].bPhysAddr[0],
                      pIpNetTable->table[i].bPhysAddr[1],
                      pIpNetTable->table[i].bPhysAddr[2],
                      pIpNetTable->table[i].bPhysAddr[3],
                      pIpNetTable->table[i].bPhysAddr[4],
                      pIpNetTable->table[i].bPhysAddr[5]) == 12);

      return mac_addr;
    }
    return "";
  }

} // namespace cyy::naive_lib::hardware
#endif
