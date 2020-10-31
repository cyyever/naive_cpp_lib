/*!
 * \file process.hpp
 *
 * \brief 封裝進程處理代碼
 * \author cyy
 */

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <gsl/gsl>
#include <sys/types.h>

namespace cyy::naive_lib::process {

  struct spawn_config {
    std::string binary_path;
    std::vector<std::string> args;
    std::vector<std::string> env{};
    bool need_channel{false};
  };

  struct spawn_result {
    pid_t pid{-1};
    std::optional<int> channel_fd{};
  };

  /// \brief 執行fork和exec
  /// \return 如果成功，返回結果
  std::optional<spawn_result> spawn(const spawn_config &config);

  namespace monitor {

    bool init();

    using prog_id = size_t;

    std::optional<prog_id>
    start_process(const spawn_config &config,
                  std::chrono::milliseconds retry_duration =
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::seconds(10)),
                  size_t max_retry_count = 10);

    bool signal_monitored_process(prog_id id, int signo);

    //! \brief 寫入指定數據到monitored process
    //! \return
    //! 實際寫入的數據量。如果系統調用失敗，直接返回失敗，不返回之前部分寫入的數據量
    std::optional<size_t>
    write_monitored_process(prog_id id, gsl::not_null<const void *> data,
                            size_t data_len);

    //! \brief 读取monitored process，最多讀取max_read_size個字節
    //! \return 實際讀取到的數據
    std::optional<std::vector<std::byte>>
    read_monitored_process(prog_id id, size_t max_read_size = SIZE_MAX);

    bool monitored_process_exist(prog_id id);
  } // namespace monitor
} // namespace cyy::naive_lib::process
