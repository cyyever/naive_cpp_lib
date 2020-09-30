/*!
 * \file hardware.cpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <mutex>

#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#endif

#include "hardware.hpp"
#include "log/log.hpp"

namespace cyy::cxx_lib::hardware {

  size_t cpu_num() { return std::thread::hardware_concurrency(); }

#ifdef HAVE_CUDA
  size_t gpu_num() {
    if (getenv("TEST_NO_CUDA")) {
      return 0;
    }
    static int _gpu_num = -1;
    static std::mutex gpu_mutex;
    if (_gpu_num >= 0) {
      return static_cast<size_t>(_gpu_num);
    }
    std::lock_guard lk(gpu_mutex);
    int device_count = 0;
    auto const error = cudaGetDeviceCount(&device_count);
    if (error != cudaSuccess) {
      if (error == cudaErrorInsufficientDriver) {
        return 0;
      }

      LOG_ERROR("cudaGetDeviceCount failed {} {},so we treat as no GPU:", error,
                cudaGetErrorString(error));
      return 0;
    }

    if (device_count < 0) {
      LOG_ERROR("invalid device_count {},so we treat as no GPU:", device_count,
                cudaGetErrorString(error));

      return 0;
    }
    _gpu_num = device_count;
    return static_cast<size_t>(_gpu_num);
  }
  int gpu_no() noexcept(false) {
    int device = 0;
    auto error = cudaGetDevice(&device);
    if (error != cudaSuccess) {
      throw std::runtime_error(std::string("cudaGetDevice failed: ") +
                               std::to_string(error) + " " +
                               cudaGetErrorString(error));
    }
    return device;
  }
#endif

  namespace {
    template <typename T> void wrap_inc(T &i, T max_i) {
      i++;
      if (i >= max_i) {
        i = 0;
      }
    }
  } // namespace

  namespace round_robin_allocator {
    size_t next_cpu_no() {
      static std::mutex cpu_mutex;
      static size_t next_cpu_no;

      std::lock_guard<std::mutex> lock(cpu_mutex);

      auto const cur_cpu_no = next_cpu_no;
      wrap_inc(next_cpu_no, cpu_num());
      return cur_cpu_no;
    }

#ifdef HAVE_CUDA
    size_t next_gpu_no() {
      static std::mutex gpu_mutex;
      static size_t next_gpu_no;

      std::lock_guard<std::mutex> lock(gpu_mutex);
      auto const cur_gpu_no = next_gpu_no;
      wrap_inc(next_gpu_no, gpu_num());
      return cur_gpu_no;
    }
#endif

  } // namespace round_robin_allocator

} // namespace cyy::cxx_lib::hardware
