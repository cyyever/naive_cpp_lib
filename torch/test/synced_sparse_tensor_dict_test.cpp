/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <doctest/doctest.h>
#include <thread>

#include "torch/synced_sparse_tensor_dict.hpp"

TEST_CASE("synced_tensor_dict") {
  auto sparse_tensor = torch::eye(3);
  auto mask = (sparse_tensor != 0);

  cyy::cxx_lib::pytorch::synced_sparse_tensor_dict dict("tensor_dir", mask,
                                                        sparse_tensor.sizes());

  CHECK_EQ(dict.size(), 0);

  dict.set_in_memory_number(3);
  dict.set_saving_thread_number(5);
  dict.set_fetch_thread_number(5);

  // save sparse tensor
  dict.emplace("sparse_tensor", sparse_tensor);

  CHECK_EQ(dict.size(), 1);
  CHECK_EQ(dict.keys().size(), 1);
  dict.erase("sparse_tensor");

  CHECK_EQ(dict.size(), 0);

  dict.disable_permanent_storage();
}
