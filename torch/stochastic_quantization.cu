#include <stdexcept>

#include <ATen/TensorIterator.h>
#include <ATen/native/cuda/Loops.cuh>
#include <ATen/native/cpu/Loops.h>

#include "stochastic_quantization.hpp"

namespace cyy::naive_lib::pytorch {

  template <typename scalar_t>
  void stochastic_quantization_kernel(at::Tensor &slot_ret,
                                      const at::Tensor &src,
                                      uint64_t quantization_level) {
    auto iter = at::TensorIteratorConfig()
                    .check_all_same_dtype(false)
                    .add_output(slot_ret)
                    .add_input(src)
                    .build();

    if (src.is_cuda()) {
      at::native::gpu_kernel(
          iter, [quantization_level] GPU_LAMBDA(const scalar_t src_val) -> float {
            uint64_t slot = static_cast<uint64_t>(static_cast<float>(src_val) *
                                                  quantization_level);
            return slot;
          });
    } else {
      at::native::cpu_kernel(
          iter, [quantization_level](const scalar_t src_val) -> float {
            uint64_t slot = static_cast<uint64_t>(static_cast<float>(src_val) *
                                                  quantization_level);
            return slot;
          });

    }
  }

  torch::Tensor stochastic_quantization(torch::Tensor normalized_abs_tensor,
                                        uint64_t quantization_level) {
    torch::Tensor slot_ret = normalized_abs_tensor.clone();
    if (quantization_level == 0) {
      throw std::invalid_argument("quantization_level must >0");
    }
    stochastic_quantization_kernel<float>(slot_ret, normalized_abs_tensor,
                                          quantization_level);

    return slot_ret;
  }
} // namespace cyy::naive_lib::pytorch
