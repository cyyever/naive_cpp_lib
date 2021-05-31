#include "stochastic_quantization.hpp"

#include <stdexcept>

namespace cyy::naive_lib::pytorch {

  torch::Tensor stochastic_quantization(torch::Tensor normalized_abs_tensor,
                                        uint64_t quantization_level) {
    if (quantization_level == 0) {
      throw std::invalid_argument("quantization_level must >0");
    }
    torch::Tensor prob_tensor = normalized_abs_tensor.clone();
    if (normalized_abs_tensor.is_cuda()) {
#ifdef HAS_CUDA
      stochastic_quantization_gpu(prob_tensor, normalized_abs_tensor,
                                  quantization_level);
#else
      throw std::runtime_error("No CUDA support");
#endif
    } else {
      stochastic_quantization_cpu(prob_tensor, normalized_abs_tensor,
                                  quantization_level);
    }
    return prob_tensor;
  }
} // namespace cyy::naive_lib::pytorch
