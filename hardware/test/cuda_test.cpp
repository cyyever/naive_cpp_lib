/*!
 * \file hardware_test.cpp
 *
 * \brief 测试硬件相关函数
 * \author cyy
 */

#include <vector>

#include <doctest/doctest.h>

#include "hardware/cuda.hpp"
#include "hardware/hardware.hpp"

#ifdef HAVE_CUDA
#ifdef TEST_GPU
TEST_CASE("gpu_num") {
  CHECK(cyy::naive_lib::hardware::gpu_num() > 0);

  auto gpu_no = cyy::naive_lib::hardware::round_robin_allocator::next_gpu_no();
  CHECK(gpu_no < cyy::naive_lib::hardware::gpu_num());
  CHECK(cyy::naive_lib::hardware::gpu_no() >= 0);
}
TEST_CASE("cuda stream") {
  cyy::naive_lib::hardware::cuda::get_copy_to_device_stream();
  cyy::naive_lib::hardware::cuda::get_copy_to_host_stream();
}
#endif
#endif
