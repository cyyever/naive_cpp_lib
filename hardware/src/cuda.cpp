/*!
 * \file hardware_cuda.cpp
 *
 * \brief 封装一些CUDA相关的接口
 * \author cyy
 */

#include "cuda.hpp"
#include "log/src/log.hpp"

#ifdef HAVE_CUDA
namespace cyy::cxx_lib::hardware::cuda {

  std::optional<cudaStream_t> create_nonblock_stream() {
    cudaStream_t stream{};
    auto error = cudaStreamCreateWithFlags(&stream, cudaStreamNonBlocking);
    if (error != cudaSuccess) {
      LOG_ERROR("cudaStreamCreateWithFlags failed {} {}", error,
                cudaGetErrorString(error));
      return {};
    }

    return {stream};
  }

  std::optional<cudaStream_t> &get_copy_to_device_stream() {
    static std::optional<cudaStream_t> to_device_stream{
        create_nonblock_stream()};
    return to_device_stream;
  }

  std::optional<cudaStream_t> &get_copy_to_host_stream() {
    static std::optional<cudaStream_t> to_host_stream{create_nonblock_stream()};
    return to_host_stream;
  }
} // namespace cyy::cxx_lib::hardware::cuda

#endif
