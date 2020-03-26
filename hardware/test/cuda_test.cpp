/*!
 * \file hardware_test.cpp
 *
 * \brief 测试硬件相关函数
 * \author cyy
 */

#include <doctest/doctest.h>
#include <vector>

#include "../src/hardware.hpp"
#include "../src/cuda.hpp"

#ifdef HAVE_CUDA
#ifdef TEST_GPU
TEST_CASE("gpu_num") {
  CHECK(deepir::hardware::gpu_num() > 0);

  auto gpu_no = deepir::hardware::round_robin_allocator::next_gpu_no();
  CHECK(gpu_no < deepir::hardware::gpu_num());
  CHECK(deepir::hardware::gpu_no() >= 0);
}
TEST_CASE("cuda stream") {
    deepir::hardware::cuda::get_copy_to_device_stream();
    deepir::hardware::cuda::get_copy_to_host_stream();
}
#endif
#endif
