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
  class neural_network_task : public base_task {
  public:
    ~neural_network_task() override = default;

    void set_model_version(const std::string &version) {
      model_version = version;
    }

    const std::string &get_model_version() { return model_version; }

  private:
    std::string model_version;
  };

  //! \brief neural_network_task子类模板，用于抽象数据和结果
  template <typename data_cls, typename res_cls>
  class neural_network_task_sub_tpl : public neural_network_task {
  public:
    neural_network_task_sub_tpl() = default;
    neural_network_task_sub_tpl(const data_cls &data_) : data(data_) {}
    neural_network_task_sub_tpl(data_cls &data_, res_cls &res_)
        : data(data_), res(res_) {}

    ~neural_network_task_sub_tpl() override = default;

    const data_cls &get_data() const { return data; }

    void set_res(const res_cls &res_) { res = res_; }

    const res_cls &get_res() const { return res; }

  protected:
    data_cls data;
    res_cls res;
  };
} // namespace cyy::cxx_lib::task
