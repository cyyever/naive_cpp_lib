/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <chrono>
#include <torch/torch.h>

#include "log/log.hpp"
#include "torch/synced_tensor_dict.hpp"
#include "util/time.hpp"
int main(int argc, char **argv) {
  cyy::cxx_lib::log::set_level(spdlog::level::level_enum::err);
  cyy::cxx_lib::pytorch::synced_tensor_dict dict("tensor_dir_profiling");

  dict.set_in_memory_number(100);
  dict.enable_permanent_storage();

  auto tensor = torch::randn({1, 200 * 1024});
  auto begin_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  for (int i = 0; i < 1024; i++) {
    dict.emplace(std::to_string(i), tensor);
  }
  dict.flush_all(true);
  auto end_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  std::cout << "insertion used " << end_ms - begin_ms << " ms" << std::endl;

  cyy::cxx_lib::pytorch::synced_tensor_dict dict2("tensor_dir_profiling");

  dict2.set_in_memory_number(1024);
  dict2.enable_permanent_storage();

  begin_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  dict2.prefetch(dict2.keys());
  for (int i = 0; i < 1024; i++) {
    tensor = dict2.get(std::to_string(i));
  }
  end_ms = cyy::cxx_lib::time::now_ms<std::chrono::steady_clock>();
  std::cout << "read used " << end_ms - begin_ms << " ms" << std::endl;
  /* dict2.clear(); */
  return 0;
}
