/*!
 * \file log.hpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#pragma once

#include <filesystem>
#include <format>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

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
                         ::spdlog::level::level_enum min_level);

  template <typename... Args>
  void log_message(const std::source_location &location,
                   spdlog::level::level_enum level,
                   std::format_string<Args...> fmt_string, Args &&...args) {
    auto message = std::format(
        "[{}:{}] {}",
        std::filesystem::path(location.file_name()).filename().string(),
        location.line(),
        std::format(fmt_string, std::forward<Args>(args)...));

    spdlog::apply_all([&](const std::shared_ptr<spdlog::logger> &logger) {
      logger->log(level, "{}", message);
    });
  }

} // namespace cyy::naive_lib::log

#define LOG_IMPL(log_level, ...)                                               \
  cyy::naive_lib::log::log_message(std::source_location::current(),            \
                                   log_level __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(...) LOG_IMPL(spdlog::level::level_enum::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG_IMPL(spdlog::level::level_enum::info, __VA_ARGS__)
#define LOG_WARN(...) LOG_IMPL(spdlog::level::level_enum::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG_IMPL(spdlog::level::level_enum::err, __VA_ARGS__)
