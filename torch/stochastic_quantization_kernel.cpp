
#include <ATen/TensorIterator.h>
#include <ATen/native/cpu/Loops.h>

namespace cyy::naive_lib::pytorch {

  void stochastic_quantization_cpu(at::Tensor &prob_tensor, const at::Tensor &src,
                                   uint64_t quantization_level) {
    auto iter = at::TensorIteratorConfig()
                    .check_all_same_dtype(false)
                    .add_output(prob_tensor)
                    .add_input(src)
                    .build();

    at::native::cpu_kernel(
        iter, [quantization_level](const float src_val) -> float {
          float tmp=src*quantization_level;
          return tmp-static_cast<uint64_t>(tmp);
        });
  }

} // namespace cyy::naive_lib::pytorch
