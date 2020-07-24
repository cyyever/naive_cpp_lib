/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

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
  auto begin_ms =now_ms();
  for (int i = 0; i < 100; i++) {
    torch::save(tensor, "tmp_file");
  }
  auto end_ms = now_ms();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;

  begin_ms = now_ms();
  for (int i = 0; i < 100; i++) {
    std::filesystem::remove("tmp_file");
    torch::save(tensor, "tmp_file");
  }
  end_ms = now_ms();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;

  return 0;
}
