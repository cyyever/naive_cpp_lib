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
  /* dict.set_saving_thread_number(10); */
  /* dict.set_fetch_thread_number(10); */

  // save sparse tensor
  auto sparse_tensor= torch::eye(3).to_sparse();
  dict.emplace("sparse_tensor",sparse_tensor);

  CHECK_EQ(dict.size(), 1);
  CHECK_EQ(dict.keys().size(), 1);
  dict.erase("sparse_tensor");

  CHECK_EQ(dict.size(), 0);


  std::vector<std::thread> thds;
  for (int i = 0; i < 10; i++) {
    thds.emplace_back([i, &dict]() {
      for (int j = 0; j < 10; j++) {
        dict.emplace(std::to_string(i * 10 + j), torch::rand({1,200*1024}));
      }
    });
  }
  for (auto &thd : thds) {
    thd.join();
  }
  thds.clear();
  CHECK_EQ(dict.size(), 100);
  CHECK_EQ(dict.keys().size(), 100);

  std::vector<std::string> keys;
  for (int i = 0; i < 10; i++) {
      for (int j = 0; j < 10; j++) {
        keys.emplace_back(std::to_string(i * 10 + j));
      }
  }
  dict.flush_all();

  dict.prefetch(keys);

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
  dict.enable_permanent_storage();
  dict.release();
  cyy::cxx_lib::pytorch::synced_tensor_dict dict2("tensor_dir");
  CHECK_EQ(dict2.size(), 100);
  dict2.clear();
  CHECK_EQ(dict2.size(), 0);
}
