#pragma once

#include "synced_tensor_dict.hpp"

namespace cyy::cxx_lib::pytorch {
  class synced_sparse_tensor_dict : public synced_tensor_dict {
  public:
    synced_sparse_tensor_dict(torch::Tensor mask_,
                              torch::IntArrayRef tensor_shape_,
                              const std::string &storage_dir_);
    ~synced_sparse_tensor_dict();
    void emplace(const std::string &key, const torch::Tensor &value);
    torch::Tensor get(const std::string &key);

  private:
    torch::Tensor mask;
    std::vector<torch::IntArrayRef::value_type> tensor_shape;
  };
} // namespace cyy::cxx_lib::pytorch
