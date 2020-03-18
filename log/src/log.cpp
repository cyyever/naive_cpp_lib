/*!
 * \file log.cpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

/* #include <spdlog/details/registry.h> */
/* #include <spdlog/sinks/rotating_file_sink.h> */
/* #include <spdlog/sinks/stdout_color_sinks.h> */
#include <spdlog/spdlog.h>

#include "log.hpp"
namespace {

  std::string now_str() {
    time_t t;
    time(&t);
    char buf[100]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", std::localtime(&t));
    return buf;
  }

  std::string get_full_path(const std::string &log_dir,
                            const std::string &name) {
    std::string log_path = log_dir;
    std::string tail = log_path.substr(log_path.size() - 1);
    if (tail != "/") {
      log_path += "/";
    }
    log_path += name;
    log_path += "-";
    log_path += now_str();
    log_path += "--";
#ifdef _WIN32
    log_path += std::to_string(_getpid());
#else
    log_path += std::to_string(getpid());
#endif
    log_path += ".log";
    return log_path;
  }

  struct init {
    init() {
      // call this function on program starup to avoid race condition
      spdlog::details::registry::instance();
    }
  } initer;
} // namespace

namespace cyy::cxx::log {

  void setup_file_logger(const std::string &log_dir, const std::string &name,
                         ::spdlog::level::level_enum level,
                         size_t max_file_size, size_t max_file_num) {
    using ::spdlog::level::level_enum;
    for (int l = static_cast<int>(level);
         l <= static_cast<int>(level_enum::err); l++) {
      auto logger_name =
          name + "-" + spdlog::level::to_str(static_cast<level_enum>(l));
      auto file_logger = ::spdlog::rotating_logger_mt(
          logger_name, get_full_path(log_dir, logger_name), max_file_size,
          max_file_num);
      file_logger->set_level(static_cast<level_enum>(l));
      file_logger->flush_on(static_cast<level_enum>(l));
      file_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f][%t][%l] %v");
    }
  }

  void setup_console_logger() {
    static auto logger1 = spdlog::stdout_color_mt(console_logger_name);
  }
} // namespace cyy::cxx::log
