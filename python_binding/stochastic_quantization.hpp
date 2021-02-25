/*!
 * \file synced_tensor_dict.hpp
 *
 * \author cyy
 */

#pragma once
#include <pybind11/pybind11.h>
#include <torch/extension.h>

#include "torch/stochastic_quantization.hpp"
namespace py = pybind11;
inline void define_torch_quantization_extension(py::module_ &m) {
  auto sub_m = m.def_submodule("torch", "Contains pytorch extension");
  sub_m.def("stochastic_quantization",
            &cyy::naive_lib::pytorch::stochastic_quantization);
}
