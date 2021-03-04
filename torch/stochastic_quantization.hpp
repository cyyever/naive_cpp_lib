#pragma once

#include <torch/types.h>

namespace cyy::naive_lib::pytorch {
  torch::Tensor stochastic_quantization(torch::Tensor normalized_abs_tensor,
                                        uint64_t quantization_level);
  torch::Tensor
  stochastic_quantization_cpu(at::Tensor &slot_tensor,
                              const torch::Tensor &normalized_abs_tensor,
                              uint64_t quantization_level);
  torch::Tensor
  stochastic_quantization_gpu(at::Tensor &slot_tensor,
                              const torch::Tensor &normalized_abs_tensor,
                              uint64_t quantization_level);
} // namespace cyy::naive_lib::pytorch
