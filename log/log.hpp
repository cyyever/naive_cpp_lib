/*!
 * \file log.hpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#pragma once

#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>
#include <version>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace cyy::naive_lib::log {

#if defined(_WIN32)
  const std::wstring &get_thread_name();
#else
  const std::string &get_thread_name();
#endif
  void set_thread_name(std::string_view name);
  void set_level(spdlog::level::level_enum level);

  void setup_file_logger(const std::filesystem::path &log_dir,
                         const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size = 512 * 1024 * 1024,
                         size_t max_file_num = 3);

  inline constexpr std::string complete_fmt(std::string fmt_str) {
    return std::string("[{}:{}] ") + fmt_str;
  }

  template <typename T, typename... Args>
  void log_message(spdlog::level::level_enum level,
                   const std::source_location &location, const T &fmt_string,
                   Args... args) {
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> logger) {
      logger->log(
          level, fmt::runtime(fmt_string),
          std::filesystem::path(location.file_name()).filename().string(),
          location.line(), args...);
    });
  }

} // namespace cyy::naive_lib::log

#define __LOG_IMPL(log_level, fmt_str, ...)                                    \
  cyy::naive_lib::log::log_message(log_level, std::source_location::current(), \
                                   cyy::naive_lib::log::complete_fmt(fmt_str)  \
                                       __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(...) __LOG_IMPL(spdlog::level::level_enum::debug, __VA_ARGS__)
#define LOG_INFO(...) __LOG_IMPL(spdlog::level::level_enum::info, __VA_ARGS__)
#define LOG_WARN(...) __LOG_IMPL(spdlog::level::level_enum::warn, __VA_ARGS__)
#define LOG_ERROR(...) __LOG_IMPL(spdlog::level::level_enum::err, __VA_ARGS__)
