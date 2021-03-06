/*!
 * \file log.hpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#pragma once

#include <filesystem>
#include <string>

#include <spdlog/spdlog.h>

#if __has_include(<source_location>)
#include <source_location>
#else
// copy from <experimental/source_location> and modify some code to make clang
// success.

namespace std {
  struct source_location {

    // 14.1.2, source_location creation
    static constexpr source_location current(
#if defined(__clang__)
        const char *__file = "unknown", int __line = 0
#elif defined(__GNUC__) || defined(__GNUG__)
        const char *__file = __builtin_FILE(), int __line = __builtin_LINE()
#else
        const char *__file = "unknown", int __line = 0
#endif
        ) noexcept {
      source_location _loc;
      _loc._M_file = __file;
      _loc._M_line = static_cast<uint_least32_t>(__line);
      return _loc;
    }

    constexpr source_location() noexcept {}

    // 14.1.3, source_location field access
    constexpr uint_least32_t line() const noexcept { return _M_line; }
    constexpr const char *file_name() const noexcept { return _M_file; }

  private:
    const char *_M_file{"unknown"};
    uint_least32_t _M_line{0};
  }
#ifdef __GNUG__
  __attribute__((aligned(16)))
#endif
  ;
} // namespace std
#endif

namespace cyy::naive_lib::log {
  struct initer {
    initer();
  };
  inline initer __initer;

  void set_level(spdlog::level::level_enum level);

  void setup_file_logger(const std::filesystem::path &log_dir,
                         const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size = 512 * 1024 * 1024,
                         size_t max_file_num = 3);

  template <typename... Args>
  void log_message(spdlog::level::level_enum level,
                   const std::source_location &location, std::string fmt,
                   Args &&...args) {
    auto real_fmt = std::string("[") + location.file_name() + ":" +
                    std::to_string(location.line()) + "] " + std::move(fmt);

    spdlog::apply_all([&](auto const &logger) {
      logger->log(level, real_fmt.c_str(), std::forward<Args>(args)...);
    });
  }

} // namespace cyy::naive_lib::log

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
