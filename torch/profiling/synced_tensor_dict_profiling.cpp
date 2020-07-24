/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <chrono>
#include <torch/torch.h>

#include "torch/synced_tensor_dict.hpp"
#include "util/time.hpp"
int main(int argc, char **argv) {
  cyy::cxx_lib::pytorch::synced_tensor_dict dict("tensor_dir_profiling");

  dict.set_in_memory_number(1024);

  auto begin_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  for (int i = 0; i < 100; i++) {
    dict.emplace(std::to_string(i), torch::zeros({1, 200 * 1024}));
  }
  auto end_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  std::cout<<"insertion used "<<end_ms-begin_ms<<" ms"<<std::endl;

  dict.disable_permanent_storage();
  dict.clear();
  return 0;
}
