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

namespace cyy::naive_lib::io {

  //! \brief 读取文件内容
  std::optional<std::vector<std::byte>>
  get_file_content(const std::filesystem::path &file_path);

  //! \brief 读取文件内容
  bool get_file_content(const std::filesystem::path &file_path,
                        std::vector<std::byte> &content);

  //! \brief 寫入指定數據到文件
  //! \return
  //! 如果fd是阻塞的，直到所有數據寫入成功才返回，否則返回實際寫入的數據量。如果系統調用失敗，直接返回失敗，不返回之前部分寫入的數據量
  std::optional<size_t> write(const std::filesystem::path &file_path,
                              const void *data, size_t data_len);

  //! \brief 寫入指定數據到文件
  //! \return
  //! 如果fd是阻塞的，直到所有數據寫入成功才返回，否則返回實際寫入的數據量。如果系統調用失敗，直接返回失敗，不返回之前部分寫入的數據量
  std::optional<size_t> write(int fd, const void *data, size_t data_len);

  //! \brief 读取文件，最多讀取max_read_size個字節
  //! \return 是否失败
  bool read(int fd, std::vector<std::byte> &buf,
            std::optional<size_t> max_read_size_opt = {});

  //! \brief 读取文件，最多讀取max_read_size個字節
  //! \return 實際讀取到的數據
  std::pair<bool, std::vector<std::byte>>
  read(int fd, std::optional<size_t> max_read_size_opt = {});

#ifndef WIN32
  class read_only_mmaped_file final {
  public:
    explicit read_only_mmaped_file(const std::filesystem::path &file_path);
    ~read_only_mmaped_file();
    read_only_mmaped_file(const read_only_mmaped_file &) = delete;
    read_only_mmaped_file &operator=(const read_only_mmaped_file &) = delete;

    read_only_mmaped_file(read_only_mmaped_file &&) noexcept = delete;
    read_only_mmaped_file &
    operator=(read_only_mmaped_file &&) noexcept = delete;

    const void *data() const { return addr; }
    size_t size() const { return file_size; }

  private:
    void *addr{nullptr};
    size_t file_size{0};
  };
#endif
} // namespace cyy::naive_lib::io
