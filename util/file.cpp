/*!
 * \file io.hpp
 *
 * \brief 封装文件IO
 * \author cyy
 * \data 2017-01-17
 */
#ifdef WIN32
#define NOMINMAX
#endif

#include <fcntl.h>
#include <fstream>

#ifndef WIN32
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "file.hpp"
#include "log/log.hpp"
#include "util/error.hpp"

import std;
namespace cyy::naive_lib::io {

  std::optional<std::vector<std::byte>>
  get_file_content(const std::filesystem::path &file_path) {
    std::vector<std::byte> file_content;
    char buffer[1024];
    std::ifstream file(file_path, std::ios::in | std::ios::binary);
    if (!file) {
      LOG_ERROR("open file failed:{}", file_path.string());
      return {};
    }
    while (true) {
      file.read(buffer, sizeof(buffer));
      if (file.gcount() <= 0) {
        break;
      }
      file_content.insert(
          file_content.end(), reinterpret_cast<std::byte *>(buffer),
          reinterpret_cast<std::byte *>(buffer) + file.gcount());
    }
    if (!file.eof()) {
      LOG_ERROR("read file failed:{}", file_path.string());
      return {};
    }
    return {file_content};
  }
  bool get_file_content(const std::filesystem::path &file_path,
                        std::vector<std::byte> &content) {
#ifdef WIN32
    auto fd = _wopen(file_path.c_str(), O_RDONLY);
#else
    auto fd = open(file_path.c_str(), O_RDONLY | O_CLOEXEC);
#endif
    if (fd < 0) {
      LOG_ERROR("open file {} failed:{}", file_path.string(),
                ::cyy::naive_lib::util::errno_to_str());
      return false;
    }
#ifdef WIN32
    auto &cleanup = _close;
#else
    auto &cleanup = close;
#endif
    if (!read(fd, content)) {

      LOG_ERROR("read file {} failed", file_path.string());
      cleanup(fd);
      return false;
    }
    cleanup(fd);
    return true;
  }

  std::optional<size_t> write(const std::filesystem::path &file_path,
                              const void *data, size_t data_len) {
#ifdef WIN32
    auto fd = _wopen(file_path.c_str(), O_CREAT | O_EXCL | O_WRONLY);
#else
    auto fd = open(file_path.c_str(), O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC,
                   S_IRWXU);
#endif
    if (fd < 0) {
      LOG_ERROR("open file {} failed:{}", file_path.string(),
                ::cyy::naive_lib::util::errno_to_str());
      return {};
    }
#ifdef WIN32
    auto &cleanup = _close;
#else
    auto &cleanup = close;
#endif
    auto res = write(fd, data, data_len);
    cleanup(fd);
    return res;
  }

  std::optional<size_t> write(int fd, const void *data, size_t data_len) {
    size_t write_cnt = 0;
    const auto *tmp_data = static_cast<const std::byte *>(data);
    while (data_len != 0) {
#ifdef WIN32
      auto cnt = ::_write(fd, tmp_data, data_len);
#else
      auto cnt = ::write(fd, tmp_data, data_len);
#endif
      if (cnt >= 0) {
        auto unsized_cnt = static_cast<size_t>(cnt);
        write_cnt += unsized_cnt;
        tmp_data += unsized_cnt;
        data_len -= unsized_cnt;
        continue;
      }

      auto saved_errno = errno;
      if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
        return {write_cnt};
      }
      if (saved_errno == EINTR) {
        continue;
      } else {
        LOG_ERROR("write failed:{}",
                  ::cyy::naive_lib::util::errno_to_str(saved_errno));
        return {};
      }
    }
    return {write_cnt};
  }

  bool read(int fd, std::vector<std::byte> &buf,
            const std::optional<size_t> &max_read_size_opt) {
    size_t max_read_size = SIZE_MAX;
    if (max_read_size_opt.has_value()) {
      max_read_size = max_read_size_opt.value();
    } else {
      struct stat sb{};
      if (fstat(fd, &sb) != 0) {
        LOG_ERROR("fstat failed:{}", ::cyy::naive_lib::util::errno_to_str());
        return false;
      }
      max_read_size = sb.st_size;
    }
    buf.resize(max_read_size);
    size_t total_cnt = 0;
    while (total_cnt < max_read_size) {
      auto read_size = max_read_size - total_cnt;
#ifdef WIN32
      auto cnt = ::_read(fd, buf.data() + total_cnt, read_size);
#else
      auto cnt = ::read(fd, buf.data() + total_cnt, read_size);
#endif
      if (cnt == 0) { // EOF
        buf.resize(total_cnt);
        break;
      }
      if (cnt > 0) {
        total_cnt += cnt;
        if (static_cast<size_t>(cnt) < read_size) { // 這意味着下次讀會阻塞
          break;
        }
        continue;
      }

      auto saved_errno = errno;
      if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
        break;
      }
      if (saved_errno == EINTR) {
        continue;
      } else {
        LOG_ERROR("read failed:{}",
                  ::cyy::naive_lib::util::errno_to_str(saved_errno));
        buf.resize(total_cnt);
        return false;
      }
    }
    return true;
  }

  std::pair<bool, std::vector<std::byte>>
  read(int fd, const std::optional<size_t> &max_read_size_opt) {
    std::vector<std::byte> buf;
    auto res = read(fd, buf, max_read_size_opt);
    return {res, std::move(buf)};
  }
#ifndef WIN32
  read_only_mmaped_file::read_only_mmaped_file(
      const std::filesystem::path &file_path) {
    auto fd = open(file_path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
      throw std::runtime_error(std::string("open failed: ") +
                               ::cyy::naive_lib::util::errno_to_str(errno));
    }
    /* Obtain the size of the file and use it to specify the size of       the
     * mapping and the size of the buffer to be written */
    struct stat sb{};
    if (fstat(fd, &sb) != 0) {
      close(fd);
      auto saved_errno = errno;
      throw std::runtime_error(
          std::string("fstat failed: ") +
          ::cyy::naive_lib::util::errno_to_str(saved_errno));
    }
    file_size = sb.st_size;
    addr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (addr == MAP_FAILED) {
      auto saved_errno = errno;
      throw std::runtime_error(
          std::string("mmap failed: ") +
          ::cyy::naive_lib::util::errno_to_str(saved_errno));
    }
  }
  read_only_mmaped_file::~read_only_mmaped_file() {
    if (munmap(addr, file_size) != 0) {
      LOG_ERROR("munmap failed:{}",
                ::cyy::naive_lib::util::errno_to_str(errno));
    }
  }
#endif

} // namespace cyy::naive_lib::io
