/*!
 * \file neural_network_processor.hpp
 *
 * \brief 神经网络任务处理器
 * \author cyy
 * \date 2016-11-09
 */
#pragma once

#include "base_processor.hpp"

namespace cyy::naive_lib::task {

  //! \brief 神经网络任务处理器
  class neural_network_processor : public base_processor {
  public:
    neural_network_processor() = default;
    ~neural_network_processor() override = default;

    void set_model_version(const std::string &version) {
      model_version = version;
    }

  protected:
    std::string model_version;
  };
} // namespace cyy::naive_lib::task
