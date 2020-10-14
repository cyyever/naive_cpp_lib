/*!
 * \file string.cpp
 *
 * \brief 封装一些字符串操作
 * \author cyy
 * \date 2017-01-17
 */

#ifdef _WIN32
#include <windows.h>
#include <Stringapiset.h>
#endif

#include <iostream>
#include <memory>

#include "log/log.hpp"
#include "string.hpp"

namespace cyy::cxx_lib::strings {

  std::vector<std::string> split(const std::string &s, char c) {
    std::string buff;
    std::vector<std::string> v;

    for (auto const &n : s) {
      if (n != c) {
        buff += n;
      } else if (n == c && !buff.empty()) {
        v.push_back(buff);
        buff.clear();
      }
    }
    if (!buff.empty()) {
      v.push_back(buff);
    }
    return v;
  }

#ifdef _WIN32
  std::string GBK_to_UTF8(const std::string &str) {

    //先转成 UTF-16
    auto wcnt = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
    if (wcnt == 0) {
      LOG_ERROR("MultiByteToWideChar failed");
      return "";
    }

    auto utf16_str = std::make_unique<WCHAR[]>(wcnt + 1);

    wcnt =
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, utf16_str.get(), wcnt);
    if (wcnt == 0) {
      LOG_ERROR("MultiByteToWideChar failed");
      return "";
    }

    utf16_str[wcnt] = '\0';

    //先计算所需要的buffer大小
    auto cnt = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                   reinterpret_cast<LPCWCH>(utf16_str.get()),
                                   wcnt, nullptr, 0, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }

    auto utf8_str = std::make_unique<char[]>(cnt + 1);

    cnt = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                              reinterpret_cast<LPCWCH>(utf16_str.get()), wcnt,
                              utf8_str.get(), cnt, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }
    utf8_str[cnt] = '\0';
    return utf8_str.get();
  }

  std::string UTF16_to_UTF8(const char *str, size_t len) {
    //先计算所需要的buffer大小
    auto cnt = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, reinterpret_cast<LPCWCH>(str),
        static_cast<int>(len / 2), nullptr, 0, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }

    auto utf8_str = std::make_unique<char[]>(cnt + 1);

    cnt = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, reinterpret_cast<LPCWCH>(str),
        static_cast<int>(len / 2), utf8_str.get(), cnt, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }
    utf8_str[cnt] = '\0';
    return utf8_str.get();
  }

  std::string UTF8_to_GBK(const std::string &str) {
    //先转成 UTF-16
    auto wcnt = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (wcnt == 0) {
      LOG_ERROR("MultiByteToWideChar failed");
      return "";
    }

    auto utf16_str = std::make_unique<WCHAR[]>(wcnt + 1);

    wcnt =
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, utf16_str.get(), wcnt);
    if (wcnt == 0) {
      LOG_ERROR("MultiByteToWideChar failed");
      return "";
    }

    utf16_str[wcnt] = '\0';

    //先计算所需要的buffer大小
    auto cnt = WideCharToMultiByte(CP_ACP, 0,
                                   reinterpret_cast<LPCWCH>(utf16_str.get()),
                                   wcnt, nullptr, 0, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }

    auto gbk_str = std::make_unique<char[]>(cnt + 1);

    cnt = WideCharToMultiByte(CP_ACP, 0,
                              reinterpret_cast<LPCWCH>(utf16_str.get()), wcnt,
                              gbk_str.get(), cnt, nullptr, nullptr);

    if (cnt == 0) {
      LOG_ERROR("WideCharToMultiByte failed");
      return "";
    }
    gbk_str[cnt] = '\0';
    return gbk_str.get();
  }
#endif

} // namespace cyy::cxx_lib::strings
