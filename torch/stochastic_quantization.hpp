#pragma once

#include <torch/types.h>

namespace cyy::naive_lib::pytorch {
  torch::Tensor stochastic_quantization(torch::Tensor normalized_abs_tensor,
                                        uint64_t quantization_level);
  void stochastic_quantization_cpu(at::Tensor &prob_tensor,
                                   const torch::Tensor &normalized_abs_tensor,
                                   uint64_t quantization_level);
#ifdef HAS_CUDA
  void stochastic_quantization_gpu(at::Tensor &prob_tensor,
                                   const torch::Tensor &normalized_abs_tensor,
                                   uint64_t quantization_level);
#endif
} // namespace cyy::naive_lib::pytorch
