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

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "file.hpp"
#include "log/log.hpp"
#include "util/error.hpp"

namespace cyy::cxx_lib::io {

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

  std::optional<size_t> write(int fd, const void *data, size_t data_len) {
    size_t write_cnt = 0;
    auto tmp_data = static_cast<const std::byte *>(data);
    while (data_len != 0) {
#ifdef WIN32
      auto cnt = ::_write(fd, tmp_data, data_len);
#else
      auto cnt = ::write(fd, tmp_data, data_len);
#endif
      if (cnt >= 0) {
        write_cnt += static_cast<size_t>(cnt);
        tmp_data += cnt;
        data_len -= static_cast<size_t>(cnt);
        continue;
      }

      auto saved_errno = errno;
      if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
        return {write_cnt};
      } else if (saved_errno == EINTR) {
        continue;
      } else {
        LOG_ERROR("write failed:{}",
                  ::cyy::cxx_lib::util::errno_to_str(saved_errno));
        return {};
      }
    }
    return {write_cnt};
  }

  std::optional<std::vector<std::byte>> read(int fd, size_t max_read_size) {
    std::vector<std::byte> data;

    std::byte buf[1024];
    while (max_read_size) {
      auto read_size = std::min(max_read_size, sizeof(buf));
#ifdef WIN32
      auto cnt = ::_read(fd, buf, read_size);
#else
      auto cnt = ::read(fd, buf, read_size);
#endif
      if (cnt == 0) { // EOF
        break;
      } else if (cnt > 0) {
        max_read_size -= static_cast<size_t>(cnt);
        data.insert(data.end(), buf, buf + cnt);
        if (static_cast<size_t>(cnt) < read_size) { //這意味着下次讀會阻塞
          break;
        }
        continue;
      }

      auto saved_errno = errno;
      if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
        break;
      } else if (saved_errno == EINTR) {
        continue;
      } else {
        LOG_ERROR("read failed:{}",
                  ::cyy::cxx_lib::util::errno_to_str(saved_errno));
        return {};
      }
    }
    return {data};
  }

} // namespace cyy::cxx_lib::io
