#include "synced_sparse_tensor_dict.hpp"

namespace cyy::cxx_lib::pytorch {
  void synced_sparse_tensor_dict::emplace(const std::string &key,
                                          const torch::Tensor &value) {

    auto sparse_value = value.sparse_mask(mask)._values();
    synced_tensor_dict::emplace(key, sparse_value);
  }
  torch::Tensor synced_sparse_tensor_dict::get(const std::string &key) {
    auto value = synced_tensor_dict::get(key);
    return sparse_coo_tensor(mask._indices(), value, value_shape).to_dense();
  }
} // namespace cyy::cxx_lib::pytorch
