/*!
 * \file hardware_cuda.hpp
 *
 * \brief 封装一些CUDA相关的接口
 * \author cyy
 */

#ifdef HAVE_CUDA
#pragma once

#include <cuda_runtime.h>
#include <optional>

namespace cyy::cxx_lib::hardware::cuda {

  std::optional<cudaStream_t> create_nonblock_stream();

  std::optional<cudaStream_t> &get_copy_to_device_stream();

  std::optional<cudaStream_t> &get_copy_to_host_stream();
} // namespace cyy::cxx_lib::hardware::cuda

#endif
