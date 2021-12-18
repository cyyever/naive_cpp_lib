/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */
#pragma once

#include <memory>
#include <string>

#include "converter.hpp"

namespace cyy::naive_lib::video {

  //! \brief 封装ffmpeg对视频流的讀操作
  class ffmpeg_converter_impl;
  class ffmpeg_converter final : public ::cyy::naive_lib::video::converter {
  public:
    ffmpeg_converter(const std::string &in_url, const std::string &out_url);

    ~ffmpeg_converter() override;

    //! \brief 轉換視頻
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    int convert() override;

  private:
    std::unique_ptr<ffmpeg_converter_impl> pimpl;
  };
} // namespace cyy::naive_lib::video
