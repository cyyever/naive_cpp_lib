/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <chrono>
#include <filesystem>
#include <torch/serialize.h>

uint64_t now_ms() {
  return static_cast<uint64_t>(
      std::chrono::time_point_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now())
          .time_since_epoch()
          .count());
}

int main(int argc, char **argv) {
  std::filesystem::path tensor_dir("pytorch_tensor_profiling_dir");
  std::filesystem::create_directory(tensor_dir);

  auto tensor = torch::randn({1, 200 * 1024});
  auto begin_ms = now_ms();
  for (int i = 0; i < 1024; i++) {
    torch::save(tensor, tensor_dir / (std::to_string(i) + ".tensor"));
  }
  auto end_ms = now_ms();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;

  begin_ms = now_ms();
  for (int i = 0; i < 1024; i++) {
    std::filesystem::remove(tensor_dir / (std::to_string(i) + ".tensor"));
    torch::save(tensor, tensor_dir / (std::to_string(i) + ".tensor"));
  }
  end_ms = now_ms();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;
  std::filesystem::remove_all(tensor_dir);
  return 0;
}
