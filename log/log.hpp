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

  template<std::size_t N>
    struct Str {
      char chars[N];
    };
  template<std::size_t N> Str(const char(&)[N]) -> Str<N>; // deduction guide
  template<std::size_t M,std::size_t N>
  constexpr std::string concat(Str<M> a,Str<N> b) {
    return std::string(a.chars)+std::string(b.chars);
  }
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
                         size_t max_file_size = 512ull * 1024ull * 1024ull,
                         size_t max_file_num = 3);

  template <auto fmt_string, typename... Args>
  void log_message( const std::source_location &location, spdlog::level::level_enum level, Args... args) {
    auto msg = fmt::format(
    fmt::runtime(  concat(Str{"[{}:{}] "},fmt_string))
        , std::filesystem::path(location.file_name()).filename().string(), location.line(), args...);

    spdlog::apply_all([&](const std::shared_ptr<spdlog::logger>& logger) {
      logger->log(
          level, "{}",msg);
    });
  }

} // namespace cyy::naive_lib::log

#define LOG_IMPL(log_level, fmt_str, ...)                                    \
  cyy::naive_lib::log::log_message<Str{fmt_str}>(std::source_location::current(),log_level __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(...) LOG_IMPL(spdlog::level::level_enum::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG_IMPL(spdlog::level::level_enum::info, __VA_ARGS__)
#define LOG_WARN(...) LOG_IMPL(spdlog::level::level_enum::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG_IMPL(spdlog::level::level_enum::err, __VA_ARGS__)
