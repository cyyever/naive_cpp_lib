#include <ATen/TensorIterator.h>
#include <ATen/native/cuda/Loops.cuh>

namespace cyy::naive_lib::pytorch {

  void stochastic_quantization_gpu(at::Tensor &prob_tensor, const at::Tensor &src,
                                   uint64_t quantization_level) {
    auto iter = at::TensorIteratorConfig()
                    .check_all_same_dtype(false)
                    .add_output(prob_tensor)
                    .add_input(src)
                    .build();

    at::native::gpu_kernel(
        iter, [quantization_level] GPU_LAMBDA(const float src_val) -> float {
          float tmp=src*quantization_level;
          return tmp-static_cast<uint64_t>(tmp);
        });
  }

} // namespace cyy::naive_lib::pytorch
