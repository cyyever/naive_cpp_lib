#pragma once

#include "synced_tensor_dict.hpp"

namespace cyy::cxx_lib::pytorch {
  class synced_sparse_tensor_dict : public synced_tensor_dict {
  public:
    synced_sparse_tensor_dict(torch::Tensor mask_,
                              torch::IntArrayRef tensor_shape_,
                              const std::string &storage_dir_)
        : synced_tensor_dict(storage_dir_), mask{std::move(mask_)},
          tensor_shape{std::move(tensor_shape_)} {
      if (!mask.is_sparse()) {
        mask = mask.to_sparse();
      }
    }
    ~synced_sparse_tensor_dict() = default;
    void emplace(const std::string &key, const torch::Tensor &value);
    torch::Tensor get(const std::string &key);

  private:
    torch::Tensor mask;
    torch::IntArrayRef tensor_shape;
  };
} // namespace cyy::cxx_lib::pytorch
