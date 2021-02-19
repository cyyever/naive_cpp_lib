/*!
 * \file ffmpeg_base.hpp
 *
 * \brief 封装ffmpeg庫相關函數
 * \author Yue Wu,cyy
 */

#pragma once

#include <string>

//! \brief 封装ffmpeg库相關函數
namespace cyy::naive_lib::video {
  class ffmpeg_base {
  public:
    ffmpeg_base();

  protected:
    std::string errno_to_str(int err);
  };
} // namespace cyy::naive_lib::video
