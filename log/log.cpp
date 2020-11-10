/*!
 * \file log.cpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#if __has_include(<unistd.h>)
#include <unistd.h>

#include <sys/types.h>
#endif
#include <ctime>

#include "log.hpp"
namespace {

  std::string now_str() {
    time_t t;
    time(&t);
    char buf[100]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", std::localtime(&t));
    return buf;
  }

  std::filesystem::path get_file_path(const std::filesystem::path &log_dir,
                                      std::string logger_name) {
    logger_name += "-";
    logger_name += now_str();
    logger_name += "--";
#ifdef _WIN32
    logger_name += std::to_string(_getpid());
#else
    logger_name += std::to_string(getpid());
#endif
    logger_name += ".log";
    return log_dir / logger_name;
  }

} // namespace

namespace cyy::naive_lib::log {

  initer::initer() {
    // call this function on program starup to avoid race condition
    spdlog::details::registry::instance();
    auto console_logger = spdlog::stdout_color_mt("cyy_cxx");
    spdlog::set_default_logger(console_logger);
    constexpr auto log_pattern = "%^[%Y-%m-%d %H:%M:%S.%f][thd %t][%l]%v%$";
    spdlog::set_pattern(log_pattern);
  }
  void setup_file_logger(const std::filesystem::path &log_dir,
                         const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size, size_t max_file_num) {
    using ::spdlog::level::level_enum;
    for (int l = static_cast<int>(level);
         l <= static_cast<int>(level_enum::err); l++) {
      auto logger_name =
          name + "-" +
          spdlog::level::to_short_c_str(static_cast<level_enum>(l));
      auto file_logger = ::spdlog::rotating_logger_mt(
          logger_name, get_file_path(log_dir, logger_name).string(), max_file_size,
          max_file_num);
      file_logger->set_level(static_cast<level_enum>(l));
      file_logger->flush_on(static_cast<level_enum>(l));
    }
  }
  void set_level(spdlog::level::level_enum level) { spdlog::set_level(level); }

} // namespace cyy::naive_lib::log
