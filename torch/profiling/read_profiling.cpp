/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include "util/file.hpp"
#include <chrono>
#include <filesystem>
#include <torch/torch.h>

uint64_t now_ms() {
  return static_cast<uint64_t>(
      std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now())
          .time_since_epoch()
          .count());
}

int main(int argc, char **argv) {
  auto tensor = torch::randn({1, 200 * 1024});
  for (int i = 0; i < 1024; i++) {
    std::filesystem::remove(std::to_string(i) + ".tensor");
    torch::save(tensor, std::to_string(i) + ".tensor");
  }

  torch::Tensor value;
  auto begin_ms = now_ms();
  for (int i = 0; i < 1024; i++) {
    torch::load(value, std::to_string(i) + ".tensor");
  }
  auto end_ms = now_ms();
  std::cout << "read used " << end_ms - begin_ms << " ms" << std::endl;

  begin_ms = now_ms();
  for (int i = 0; i < 1024; i++) {
    cyy::cxx_lib::io::read_only_mmaped_file f(std::to_string(i) + ".tensor");
    torch::load(value, reinterpret_cast<const char *>(f.data()), f.size());
  }
  end_ms = now_ms();
  std::cout << "mmap read used " << end_ms - begin_ms << " ms" << std::endl;

  begin_ms = now_ms();
  for (int i = 0; i < 1024; i++) {
    std::vector<std::byte> buf;
    cyy::cxx_lib::io::get_file_content(std::to_string(i) + ".tensor",buf);
    torch::load(value, reinterpret_cast<const char *>(buf.data()), buf.size());
  }
  end_ms = now_ms();
  std::cout << "improved read used " << end_ms - begin_ms << " ms" << std::endl;
  return 0;
}
