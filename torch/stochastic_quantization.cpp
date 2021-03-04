#include "stochastic_quantization.hpp"

#include <stdexcept>
/* #include "stochastic_quantization_kernel.hpp" */
/* #include "stochastic_quantization_kernel.cuh" */

namespace cyy::naive_lib::pytorch {

  torch::Tensor stochastic_quantization(torch::Tensor normalized_abs_tensor,
                                        uint64_t quantization_level) {
    if (quantization_level == 0) {
      throw std::invalid_argument("quantization_level must >0");
    }
    torch::Tensor slot_ret = normalized_abs_tensor.clone();
    if (normalized_abs_tensor.is_cuda()) {
      stochastic_quantization_gpu(slot_ret, normalized_abs_tensor,
                                  quantization_level);

    } else {
      stochastic_quantization_cpu(slot_ret, normalized_abs_tensor,
                                  quantization_level);
      /* stochastic_quantization_cpu_kernel<float>(slot_ret,
       * normalized_abs_tensor, */
      /*     quantization_level); */
    }
    return slot_ret;
  }
} // namespace cyy::naive_lib::pytorch
