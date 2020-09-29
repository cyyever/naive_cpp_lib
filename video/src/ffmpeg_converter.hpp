/*!
 * \file ffmpeg_converter.h
 *
 * \brief 封装ffmpeg对视频流的轉換
 * \author Yue Wu,cyy
 */
#pragma once

#include "converter.hpp"
#include <memory>

namespace cyy::cxx_lib::video::ffmpeg {

  //! \brief 封装ffmpeg对视频流的讀操作
  class converter final : public ::cyy::cxx_lib::video::converter {
  public:
    converter(const std::string &in_url, const std::string &out_url);

    ~converter() override;

    //! \brief 轉換視頻
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    int convert() override;

  private:
    class impl;
    std::unique_ptr<impl> pimpl;
  };
} // namespace cyy::cxx_lib::video::ffmpeg
