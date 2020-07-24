/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <chrono>
#include <torch/torch.h>

#include "log/log.hpp"
#include "util/time.hpp"
int main(int argc, char **argv) {

  auto begin_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  auto tensor = torch::randn({1, 200 * 1024});
  for (int i = 0; i < 100; i++) {
    torch::save(value, "tmp_file");
  }
  auto end_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;

  return 0;
}
