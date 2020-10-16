/*!
 * \file neural_network_task.hpp
 *
 * \brief 神经网络任务
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include "base_task.hpp"

namespace cyy::cxx_lib::task {

  //! \brief 神经网络任务
  template <typename ArgumentType, typename ResultType = void>
  class neural_network_task
      : public task_with_argument_and_result<ArgumentType, ResultType> {
  public:
    using task_with_argument_and_result<
        ArgumentType, ResultType>::task_with_argument_and_result;
    ~neural_network_task() override = default;

  public:
    std::string model_version;
  };

} // namespace cyy::cxx_lib::task
