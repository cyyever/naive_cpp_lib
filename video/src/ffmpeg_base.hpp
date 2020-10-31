/*!
 * \file ffmpeg_base.hpp
 *
 * \brief 封装ffmpeg庫相關函數
 * \author Yue Wu,cyy
 */

#pragma once

#include <string>

//! \brief 封装ffmpeg库相關函數
namespace cyy::naive_lib::video::ffmpeg {
  //! \brief 初始化ffmpeg库
  void init_library();
  //! \brief 给定错误号，获取ffmpeg的错误字符串
  std::string errno_to_str(int err);
} // namespace cyy::naive_lib::video::ffmpeg
