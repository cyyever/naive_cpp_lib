/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <torch/torch.h>

#include "torch/synced_tensor_dict.hpp"
int main(int argc, char **argv) {
  cyy::cxx_lib::pytorch::synced_tensor_dict dict("tensor_dir_profiling");

  dict.set_in_memory_number(1024);

  for (int i = 0; i < 100; i++) {
    dict.emplace(std::to_string(i), torch::zeros({1, 200 * 1024}));
  }

  dict.disable_permanent_storage();
  dict.clear();
  return 0;
}
