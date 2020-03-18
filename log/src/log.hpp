/*!
 * \file log.hpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#pragma once

#include <spdlog/spdlog.h>
#include <string>

#if __has_include(<source_location>)
#include <source_location>
#else
// copy from <experimental/source_location> and modify some code to make clang
// success.

namespace std {
  struct source_location {

    // 14.1.2, source_location creation
    static constexpr source_location current(
#ifdef __GNUG__
        const char *__file = __builtin_FILE(), int __line = __builtin_LINE()
#else
        const char *__file = "unknown", int __line = 0
#endif
            ) noexcept {
      source_location __loc;
      __loc._M_file = __file;
      __loc._M_line = __line;
      return __loc;
    }

    constexpr source_location() noexcept : _M_file("unknown"), _M_line(0) {}

    // 14.1.3, source_location field access
    constexpr uint_least32_t line() const noexcept { return _M_line; }
    constexpr const char *file_name() const noexcept { return _M_file; }

  private:
    const char *_M_file;
    uint_least32_t _M_line;
  };
} // namespace std
#endif

namespace cyy::cxx_lib::log {

  inline std::string console_logger_name = "cyy_cxx";

  void setup_console_logger();

  void setup_file_logger(const std::string &log_dir, const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size = 512 * 1024 * 1024,
                         size_t max_file_num = 3);

  template <typename... Args>
  void log_message(spdlog::level::level_enum level,
                   const std::source_location &location, std::string fmt,
                   Args &&... args) {
    setup_console_logger();

    auto real_fmt = std::string(" [ ") + location.file_name() + ":" +
                    std::to_string(location.line()) + " ] " + std::move(fmt);

    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> logger) {
      logger->log(level, real_fmt.c_str(), std::forward<Args>(args)...);
    });
  }

} // namespace cyy::cxx_lib::log

#define LOG_DEBUG(...)                                                         \
  cyy::cxx_lib::log::log_message(spdlog::level::level_enum::debug,             \
                                 std::source_location::current(), __VA_ARGS__)

#define LOG_INFO(...)                                                          \
  cyy::cxx_lib::log::log_message(spdlog::level::level_enum::info,              \
                                 std::source_location::current(), __VA_ARGS__)

#define LOG_WARN(...)                                                          \
  cyy::cxx_lib::log::log_message(spdlog::level::level_enum::warn,              \
                                 std::source_location::current(), __VA_ARGS__)

#define LOG_ERROR(...)                                                         \
  cyy::cxx_lib::log::log_message(spdlog::level::level_enum::err,               \
                                 std::source_location::current(), __VA_ARGS__)
