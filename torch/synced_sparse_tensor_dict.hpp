#pragma once

#include "synced_tensor_dict.hpp"

namespace cyy::cxx_lib::pytorch {
  class synced_sparse_tensor_dict : public synced_tensor_dict {
  public:
    synced_sparse_tensor_dict(const std::string &storage_dir_,
                              torch::Tensor mask_,
                              torch::IntArrayRef value_shape_)
        : synced_tensor_dict(storage_dir_), mask{std::move(mask_)},
          value_shape{std::move(value_shape_)} {
      if (!mask.is_sparse()) {
        mask = mask.to_sparse();
      }
    }
    ~synced_sparse_tensor_dict() = default;
    void emplace(const std::string &key, const torch::Tensor &value);
    torch::Tensor get(const std::string &key);

  private:
    torch::Tensor mask;
    torch::IntArrayRef value_shape;
  };
} // namespace cyy::cxx_lib::pytorch
