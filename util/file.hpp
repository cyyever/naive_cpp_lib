/*!
 * \file file.hpp
 *
 * \brief 封装文件IO
 * \author cyy
 * \date 2017-01-17
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cyy::cxx_lib::io {

  //! \brief 读取文件内容
  std::optional<std::vector<std::byte>>
  get_file_content(const std::filesystem::path &file_path);

  //! \brief 寫入指定數據到文件
  //! \return
  //! 如果fd是阻塞的，直到所有數據寫入成功才返回，否則返回實際寫入的數據量。如果系統調用失敗，直接返回失敗，不返回之前部分寫入的數據量
  std::optional<size_t> write(int fd, const void *data, size_t data_len);

  //! \brief 读取文件，最多讀取max_read_size個字節
  //! \return 實際讀取到的數據
  std::optional<std::vector<std::byte>> read(int fd,
                                             size_t max_read_size = SIZE_MAX);

} // namespace cyy::cxx_lib::io
