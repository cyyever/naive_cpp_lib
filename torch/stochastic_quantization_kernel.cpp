
#include <ATen/TensorIterator.h>
#include <ATen/native/cpu/Loops.h>

namespace cyy::naive_lib::pytorch {

  /* template <typename scalar_t> */
  void stochastic_quantization_cpu(at::Tensor &slot_ret, const at::Tensor &src,
                                   uint64_t quantization_level) {
    auto iter = at::TensorIteratorConfig()
                    .check_all_same_dtype(false)
                    .add_output(slot_ret)
                    .add_input(src)
                    .build();

    at::native::cpu_kernel(
        iter, [quantization_level](const float src_val) -> float {
          uint64_t slot = static_cast<uint64_t>(src_val * quantization_level);
          return slot;
        });
  }

} // namespace cyy::naive_lib::pytorch
