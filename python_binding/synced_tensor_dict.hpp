/*!
 * \file synced_tensor_dict.hpp
 *
 * \author cyy
 */

#pragma once
#include <pybind11/pybind11.h>
#include <torch/extension.h>

#include "torch/synced_sparse_tensor_dict.hpp"
#include "torch/synced_tensor_dict.hpp"
namespace py = pybind11;
inline void define_torch_data_structure_extension(py::module_ &m) {
  using synced_tensor_dict = cyy::naive_lib::pytorch::synced_tensor_dict;
  using synced_sparse_tensor_dict =
      cyy::naive_lib::pytorch::synced_sparse_tensor_dict;
  auto sub_m = m.def_submodule("data_structure", "Contains data structures");
  py::class_<synced_tensor_dict>(sub_m, "SyncedTensorDict")
      .def(py::init<const std::string &>())
      .def("prefetch",
           static_cast<void (synced_tensor_dict::*)(
               const std::vector<std::string> &keys)>(
               &synced_tensor_dict::prefetch),
           py::call_guard<py::gil_scoped_release>())
      .def("set_in_memory_number", &synced_tensor_dict::set_in_memory_number)
      .def("get_in_memory_number", &synced_tensor_dict::get_in_memory_number)
      .def("set_storage_dir", &synced_tensor_dict::set_storage_dir,
           py::call_guard<py::gil_scoped_release>())
      .def("get_storage_dir", &synced_tensor_dict::get_storage_dir)
      .def("set_permanent_storage",
           &synced_tensor_dict::enable_permanent_storage)
      .def("enable_permanent_storage",
           &synced_tensor_dict::enable_permanent_storage)
      .def("disable_permanent_storage",
           &synced_tensor_dict::disable_permanent_storage)
      .def("set_wait_flush_ratio", &synced_tensor_dict::set_wait_flush_ratio)
      .def("set_saving_thread_number",
           &synced_tensor_dict::set_saving_thread_number,
           py::call_guard<py::gil_scoped_release>())
      .def("set_fetch_thread_number",
           &synced_tensor_dict::set_fetch_thread_number,
           py::call_guard<py::gil_scoped_release>())
      .def("set_logging", &synced_tensor_dict::set_logging)
      .def("__setitem__", &synced_tensor_dict::emplace,
           py::call_guard<py::gil_scoped_release>())
      .def("__len__", &synced_tensor_dict::size)
      .def("__contains__", &synced_tensor_dict::contains)
      .def("__getitem__", &synced_tensor_dict::get,
           py::call_guard<py::gil_scoped_release>())
      .def("__delitem__", &synced_tensor_dict::erase)
      .def("keys", &synced_tensor_dict::keys)
      .def("in_memory_keys", &synced_tensor_dict::in_memory_keys)
      .def("release", &synced_tensor_dict::release,
           py::call_guard<py::gil_scoped_release>())
      .def("clear", &synced_tensor_dict::clear)
      .def("__copy__", [](const synced_tensor_dict &self) { return self; })
      .def(
          "__deepcopy__",
          [](const synced_tensor_dict &self, py::dict) { return self; },
          py::arg("memo"))
      .def("flush_all", &synced_tensor_dict::flush_all,
           "flush all in-memory data to the disk", py::arg("wait") = true,
           py::call_guard<py::gil_scoped_release>())
      .def("flush", static_cast<void (synced_tensor_dict::*)(size_t)>(
                        &synced_tensor_dict::flush));
  py::class_<synced_sparse_tensor_dict, synced_tensor_dict>(
      sub_m, "SyncedSparseTensorDict")
      .def(py::init<torch::Tensor, torch::IntArrayRef, const std::string &>())
      .def("__copy__",
           [](const synced_sparse_tensor_dict &self) { return self; })
      .def(
          "__deepcopy__",
          [](const synced_sparse_tensor_dict &self, py::dict) { return self; },
          py::arg("memo"))
      .def("__getitem__", &synced_sparse_tensor_dict::get,
           py::call_guard<py::gil_scoped_release>())
      .def("__setitem__", &synced_sparse_tensor_dict::emplace,
           py::call_guard<py::gil_scoped_release>());
}
