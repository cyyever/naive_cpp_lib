/*!
 * \file converter.hpp
 *
 * \author yuewu,cyy
 */

#pragma once

namespace cyy::naive_lib::video {

  //! brief 视频轉換接口类
  class converter {
  public:
    converter() = default;

    converter(const converter &) = delete;
    converter &operator=(const converter &) = delete;

    converter(converter &&) = default;
    converter &operator=(converter &&) = default;

    virtual ~converter() = default;

    //! \brief 轉換視頻
    //! \return >0 成功
    //	      =0 EOF
    //	      <0 失敗
    virtual int convert() = 0;
  };
} // namespace cyy::naive_lib::video
