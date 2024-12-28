/*!
 * \file log.cpp
 *
 * \brief 封装日誌
 * \date 2016-04-18
 */

#include "log.hpp"

#include <ctime>

#include <spdlog/fmt/chrono.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/std.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
namespace {

  std::filesystem::path get_file_path(const std::filesystem::path &log_dir,
                                      std::string logger_name) {
    auto tp = time(nullptr);
    return log_dir / fmt::format("{}-{:%Y-%m-%d-%H-%M-%S}-{}.log", logger_name,
                                 fmt::localtime(tp),
                                 std::this_thread::get_id());
  }

} // namespace

namespace cyy::naive_lib::log {

#if defined(_WIN32)
  const std::wstring &get_thread_name() {
    static thread_local std::wstring thd_name(32, {});
#else
  const std::string &get_thread_name() {
    static thread_local std::string thd_name(32, {});
#endif
    if (thd_name[0] != '\0') {
      return thd_name;
    }
#if defined(__linux__) || defined(__FreeBSD__)
    pthread_getname_np(pthread_self(), thd_name.data(), thd_name.size());
    if (thd_name[0] == '\0') {
      thd_name = fmt::format("{}", std::this_thread::get_id());
    }
#elif defined(_WIN32)
    PWSTR data;
    const auto hr = GetThreadDescription(GetCurrentThread(), &data);
    if (SUCCEEDED(hr)) {
      thd_name = std::wstring(data);
      LocalFree(data);
    }
    if (thd_name[0] == '\0') {
      auto tmp = fmt::format("{}", GetCurrentThreadId());
      thd_name = std::wstring(tmp.begin(), tmp.end());
    }
#endif
    return thd_name;
  }
  void set_thread_name(std::string_view name) {
#if defined(__linux__) || defined(__FreeBSD__)
    // glibc has a limit on name length
    // NOLINTNEXTLINE(*magic*)
    name = name.substr(0, 15);
    pthread_setname_np(pthread_self(), std::string(name).c_str());
#elif defined(_WIN32)
    std::wstring tmp_name(name.begin(), name.end());
    SetThreadDescription(GetCurrentThread(), tmp_name.c_str());
#endif
  }

  class thread_name_formatter : public spdlog::custom_flag_formatter {
  public:
    void format(const spdlog::details::log_msg &, const std::tm &,
                spdlog::memory_buf_t &dest) override {
      dest.append(get_thread_name());
    }

    [[nodiscard]] std::unique_ptr<spdlog::custom_flag_formatter>
    clone() const override {
      return spdlog::details::make_unique<thread_name_formatter>();
    }
  };

  struct initer {
    initer() noexcept {
      // call this function on program starup to avoid race condition
      spdlog::details::registry::instance();
      auto console_logger = spdlog::stdout_color_mt("cyy_cxx");
      spdlog::set_default_logger(console_logger);
      auto formatter = std::make_unique<spdlog::pattern_formatter>();
      formatter->add_flag<thread_name_formatter>('T').set_pattern(
          "%^[%Y-%m-%d %H:%M:%S.%f][thd %T][%l]%v%$");
      spdlog::set_formatter(std::move(formatter));
    }
  };
  const static initer initer_object;
  void setup_file_logger(const std::filesystem::path &log_dir,
                         const std::string &name,
                         ::spdlog::level::level_enum min_level) {
    using ::spdlog::level::level_enum;
    // NOLINTNEXTLINE(*magic-number*)
    size_t max_file_size = 512ULL * 1024 * 1024 * 1024;
    size_t max_files = 3;
    for (int level = static_cast<int>(min_level);
         level <= static_cast<int>(level_enum::err); level++) {
      auto logger_name =
          name + "-" +
          spdlog::level::to_short_c_str(static_cast<level_enum>(level));
      auto file_logger = ::spdlog::rotating_logger_mt(
          logger_name, get_file_path(log_dir, logger_name).string(),
          max_file_size, max_files);
      file_logger->set_level(static_cast<level_enum>(level));
      file_logger->flush_on(static_cast<level_enum>(level));
    }
  }
  void set_level(spdlog::level::level_enum level) { spdlog::set_level(level); }

} // namespace cyy::naive_lib::log
