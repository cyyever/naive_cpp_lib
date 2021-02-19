/*!
 * \file hardware.cpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#ifdef _WIN32
#include <Commctrl.h>
#include <Wbemidl.h>
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
#include <winsock2.h>
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

  std::string UUID() {
    // std::unique_lock<std::mutex> lk(windows_hardware_mutex);
    static std::string UUID;
    static bool invalid_UUID;

    if (invalid_UUID) {
      return {};
    }

    if (!UUID.empty()) {
      return UUID;
    }

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
    COM_initer initer;

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    auto res = CoInitializeSecurity(
        nullptr,
        -1,                          // COM authentication
        nullptr,                     // Authentication services
        nullptr,                     // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        nullptr,                     // Authentication info
        EOAC_NONE,                   // Additional capabilities
        nullptr                      // Reserved
    );

    if (res != S_OK && res != RPC_E_TOO_LATE) {
      throw std::runtime_error("CoInitializeSecurity failed");
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = nullptr;

    res = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                           IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(res)) {
      throw std::runtime_error("CoCreateInstance failed");
    }

    ::std::unique_ptr<IWbemLocator, void (*)(IWbemLocator *)> IWbemLocator_RAII(
        pLoc, [](IWbemLocator *p) { p->Release(); });

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = nullptr;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    res = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        nullptr,                 // User name. nullptr = current user
        nullptr,                 // User password. nullptr = current
        0,                       // Locale. nullptr indicates current
        0,                       // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(res)) {

      throw std::runtime_error("ConnectServer failed");
    }

    ::std::unique_ptr<IWbemServices, void (*)(IWbemServices *)>
        IWbemServices_RAII(pSvc, [](IWbemServices *p) { p->Release(); });

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    res = CoSetProxyBlanket(pSvc,              // Indicates the proxy to set
                            RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                            RPC_C_AUTHZ_NONE,  // RPC_C_AUTHZ_xxx
                            nullptr,           // Server principal name
                            RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
                            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                            nullptr,                     // client identity
                            EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(res)) {
      throw std::runtime_error("CoSetProxyBlanket failed");
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----
    IEnumWbemClassObject *pEnumerator = nullptr;

    res = pSvc->ExecQuery(bstr_t("WQL"),
                          bstr_t("SELECT * FROM Win32_ComputerSystemProduct"),
                          WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                          nullptr, &pEnumerator);

    if (FAILED(res)) {

      throw std::runtime_error("ExecQuery Win32_ComputerSystemProduct failed");
    }

    if (!pEnumerator) {

      return {};
    } else {

      IWbemClassObject *pclsObj = nullptr;

      ULONG uReturn = 0;
      res = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

      if (FAILED(res) || uReturn == 0) {
        return {};
      }

      VARIANT vtProp;

      // Get the value of the Name property

      res = pclsObj->Get(L"UUID", 0, &vtProp, 0, 0);
      if (FAILED(res)) {
        return {};
      }
      UUID = ::cyy::naive_lib::strings::UTF16_to_UTF8(
          reinterpret_cast<char *>(vtProp.bstrVal),
          ::SysStringLen(vtProp.bstrVal));
      //去除空格
      while (!UUID.empty() && UUID.back() == ' ') {
        UUID.pop_back();
      }
      while (!UUID.empty() && UUID.front() == ' ') {
        UUID.erase(UUID.begin());
      }
      VariantClear(&vtProp);

      pclsObj->Release();
      pEnumerator->Release();
    }

    // If a UUID is not available, a UUID of all zeros is used.
    invalid_UUID = true;
    for (auto const c : UUID) {
      if (c != '-' && c != '0') {
        invalid_UUID = false;
        break;
      }
    }
    if (invalid_UUID) {
      return {};
    }
    return UUID;
  }

  std::string disk_serial_number() {
    // std::unique_lock<std::mutex> lk(windows_hardware_mutex);
    static std::string disk_serial_number;

    if (!disk_serial_number.empty()) {
      return disk_serial_number;
    }

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
    COM_initer initer;

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    auto res = CoInitializeSecurity(
        nullptr,
        -1,                          // COM authentication
        nullptr,                     // Authentication services
        nullptr,                     // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        nullptr,                     // Authentication info
        EOAC_NONE,                   // Additional capabilities
        nullptr                      // Reserved
    );

    if (res != S_OK && res != RPC_E_TOO_LATE) {
      throw std::runtime_error("CoInitializeSecurity failed");
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = nullptr;

    res = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                           IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(res)) {
      throw std::runtime_error("CoCreateInstance failed");
    }

    ::std::unique_ptr<IWbemLocator, void (*)(IWbemLocator *)> IWbemLocator_RAII(
        pLoc, [](IWbemLocator *p) { p->Release(); });

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = nullptr;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    res = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        nullptr,                 // User name. nullptr = current user
        nullptr,                 // User password. nullptr = current
        0,                       // Locale. nullptr indicates current
        0,                       // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(res)) {

      throw std::runtime_error("ConnectServer failed");
    }

    ::std::unique_ptr<IWbemServices, void (*)(IWbemServices *)>
        IWbemServices_RAII(pSvc, [](IWbemServices *p) { p->Release(); });

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    res = CoSetProxyBlanket(pSvc,              // Indicates the proxy to set
                            RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
                            RPC_C_AUTHZ_NONE,  // RPC_C_AUTHZ_xxx
                            nullptr,           // Server principal name
                            RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
                            RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                            nullptr,                     // client identity
                            EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(res)) {
      throw std::runtime_error("CoSetProxyBlanket failed");
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    IEnumWbemClassObject *pEnumerator = nullptr;
    res = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_DiskPartition where BootPartition = true"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
        &pEnumerator);

    if (FAILED(res)) {

      throw std::runtime_error("ExecQuery Win32_DiskPartition failed");
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    uint64_t disk_index = 0;
    if (!pEnumerator) {
      return {};
    } else {

      IWbemClassObject *pclsObj = nullptr;

      ULONG uReturn = 0;
      res = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

      if (FAILED(res) || uReturn == 0) {
        return {};
      }

      VARIANT vtProp;

      // Get the value of the Name property

      res = pclsObj->Get(L"diskindex", 0, &vtProp, 0, 0);
      if (FAILED(res)) {
        return {};
      }
      disk_index = vtProp.llVal;
      VariantClear(&vtProp);

      pclsObj->Release();
      pEnumerator->Release();
    }

    res = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t((std::string("SELECT * FROM Win32_DiskDrive where index =") +
                std::to_string(disk_index))
                   .c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
        &pEnumerator);

    if (FAILED(res)) {
      throw std::runtime_error("ExecQuery Win32_DiskDrive failed");
    }

    if (!pEnumerator) {
      return {};
    } else {

      IWbemClassObject *pclsObj = nullptr;

      ULONG uReturn = 0;
      res = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

      if (FAILED(res) || uReturn == 0) {
        return {};
      }

      VARIANT vtProp;

      // Get the value of the Name property

      res = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
      if (FAILED(res)) {
        return {};
      }
      disk_serial_number = ::cyy::naive_lib::strings::UTF16_to_UTF8(
          reinterpret_cast<char *>(vtProp.bstrVal),
          ::SysStringLen(vtProp.bstrVal));
      //去除空格
      while (!disk_serial_number.empty() && disk_serial_number.back() == ' ') {
        disk_serial_number.pop_back();
      }
      while (!disk_serial_number.empty() && disk_serial_number.front() == ' ') {
        disk_serial_number.erase(disk_serial_number.begin());
      }
      VariantClear(&vtProp);

      pclsObj->Release();
      pEnumerator->Release();
    }
    return disk_serial_number;
  }

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
