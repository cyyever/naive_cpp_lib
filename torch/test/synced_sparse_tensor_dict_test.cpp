/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */
#include <thread>

#include <doctest/doctest.h>

#include "torch/synced_sparse_tensor_dict.hpp"

TEST_CASE("synced_sparse_tensor_dict") {
  auto sparse_tensor = torch::eye(3);
  auto mask = (sparse_tensor != 0);

  cyy::naive_lib::pytorch::synced_sparse_tensor_dict dict(
      mask, sparse_tensor.sizes(), "");

  CHECK_EQ(dict.size(), 0);

  dict.set_in_memory_number(3);

  // save sparse tensor
  dict.emplace("sparse_tensor", sparse_tensor);
  auto sparse_tensor2 = dict.get("sparse_tensor");
  CHECK(torch::equal(sparse_tensor, sparse_tensor2));
  CHECK_EQ(dict.size(), 1);
  CHECK_EQ(dict.keys().size(), 1);
  dict.erase("sparse_tensor");

  CHECK_EQ(dict.size(), 0);

  dict.disable_permanent_storage();
}
