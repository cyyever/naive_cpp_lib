/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <doctest/doctest.h>
#include <thread>

#include "torch/synced_tensor_dict.hpp"
#include <torch/torch.h>

TEST_CASE("synced_tensor_dict") {
  cyy::cxx_lib::pytorch::synced_tensor_dict dict("tensor_dir");

  CHECK_EQ(dict.size(), 0);

  dict.set_in_memory_number(3);
  std::vector<std::thread> thds;
  for (int i = 0; i < 10; i++) {
    thds.emplace_back([i, &dict]() {
      for (int j = 0; j < 10; j++) {
        dict.emplace(std::to_string(i * 10 + j), torch::eye(3));
      }
    });
  }
  for (auto &thd : thds) {
    thd.join();
  }
  thds.clear();
  CHECK_EQ(dict.size(), 100);
  for (int i = 0; i < 10; i++) {
    thds.emplace_back([i, &dict]() {
      for (int j = 0; j < 10; j++) {
        auto tr = dict.get(std::to_string(i * 10 + j));
      }
    });
  }
  for (auto &thd : thds) {
    thd.join();
  }
  CHECK_EQ(dict.size(), 100);

  dict.clear();
  CHECK_EQ(dict.size(), 0);

  // save sparse tensor
  auto sparse_tensor= torch::eye(3).to_sparse();
  dict.emplace("sparse tensor",sparse_tensor);

  CHECK_EQ(dict.size(), 1);
  dict.set_permanent_storage();

  dict.release();
  CHECK_EQ(dict.size(), 0);
}
