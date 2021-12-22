/*!
 * \file hardware.hpp
 *
 * \brief 封装一些硬件相关的接口
 * \author cyy
 * \date 2016-09-13
 */

#pragma once

#include <optional>
#include <set>
#include <string>
#include <thread>

namespace cyy::naive_lib::hardware {

  //! \brief 获取cpu数量
  size_t cpu_num();

#ifdef HAVE_CUDA
  //! \brief 获取gpu数量
  size_t gpu_num();

  //! \brief 获取线程绑定的gpu
  int gpu_no() noexcept(false);
#endif

  //! \brief 輪流分配
  namespace round_robin_allocator {

    //! \brief 分配下一个cpu
    size_t next_cpu_no();

//! \brief 分配下一个gpu
#ifdef HAVE_CUDA
    size_t next_gpu_no();
#endif

  } // namespace round_robin_allocator

#if defined(__linux__)
  //! \brief 获取物理内存的大小，以字节为单位
  size_t memory_size() noexcept(false);

  //! \brief 给线程绑定cpu
  void alloc_cpu(std::thread &thd, size_t cpu_no);
#endif


} // namespace cyy::naive_lib::hardware
