/*!
 * \file log.hpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#pragma once

#include <filesystem>
#ifdef __cpp_lib_source_location
#include <source_location>
#endif
#include <string>
#include <string_view>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace cyy::naive_lib::log {

  const std::string &get_thread_name();
  void set_thread_name(std::string_view name);
  void set_level(spdlog::level::level_enum level);

  void setup_file_logger(const std::filesystem::path &log_dir,
                         const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size = 512 * 1024 * 1024,
                         size_t max_file_num = 3);

  template <typename... Args>
  void log_message(spdlog::level::level_enum level,
#ifdef __cpp_lib_source_location
                   const std::source_location &location,
#endif
                   std::string fmt, Args &&...args) {
    auto real_fmt = std::string("[")
#ifdef __cpp_lib_source_location
                    + location.file_name() + ":" +
                    std::to_string(location.line()) + "] "
#endif
                    + std::move(fmt);
    spdlog::apply_all([&](auto const &logger) {
      logger->log(level, fmt::runtime(real_fmt), std::forward<Args>(args)...);
    });
  }

} // namespace cyy::naive_lib::log

#ifdef __cpp_lib_source_location
#define LOG_DEBUG(...)                                                         \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::debug,           \
                                   std::source_location::current(),            \
                                   __VA_ARGS__)

#define LOG_INFO(...)                                                          \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::info,            \
                                   std::source_location::current(),            \
                                   __VA_ARGS__)

#define LOG_WARN(...)                                                          \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::warn,            \
                                   std::source_location::current(),            \
                                   __VA_ARGS__)

#define LOG_ERROR(...)                                                         \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::err,             \
                                   std::source_location::current(),            \
                                   __VA_ARGS__)
#else
#define LOG_DEBUG(...)                                                         \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::debug,           \
                                   __VA_ARGS__)

#define LOG_INFO(...)                                                          \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::info, __VA_ARGS__)

#define LOG_WARN(...)                                                          \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::warn, __VA_ARGS__)

#define LOG_ERROR(...)                                                         \
  cyy::naive_lib::log::log_message(spdlog::level::level_enum::err, __VA_ARGS__)
#endif
