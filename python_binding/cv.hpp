/*!
 * \author cyy
 */

#pragma once
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "cv/mat.hpp"
namespace py = pybind11;

namespace pybind11 {
  namespace detail {
    template <> struct type_caster<cv::Scalar> {
    public:
      /**
       * This macro establishes the name 'inty' in
       * function signatures and declares a local variable
       * 'value' of type inty
       */
      PYBIND11_TYPE_CASTER(cv::Scalar, _("CVScalar"));

      /**
       * Conversion part 1 (Python->C++): convert a PyObject into a scalar
       * instance or return false upon failure. The second argument
       * indicates whether implicit conversions should be applied.
       */
      bool load(handle src, bool) { return false; }

      /**
       * Conversion part 2 (C++ -> Python): convert an scalar instance into
       * a Python object. The second and third arguments are used to
       * indicate the return value policy and parent object (for
       * ``return_value_policy::reference_internal``) and are generally
       * ignored by implicit casters.
       */
      static handle cast(const cv::Scalar &scalar,
                         return_value_policy /* policy */,
                         handle /* parent */) {
        return py::make_tuple(scalar[0], scalar[1], scalar[2], scalar[3])
            .release();
      }
    };
  } // namespace detail
} // namespace pybind11

inline void define_cv_extension(py::module_ &m) {
  auto sub_m = m.def_submodule("cv", "OpenCV Wrapper");
  py::class_<cv::Mat>(sub_m, "Matrix", py::buffer_protocol())
      .def(py::init([](py::buffer mat) {
        /* Request a buffer descriptor from Python */
        py::buffer_info info = mat.request();
        std::vector<int> shape{static_cast<int>(info.shape[0]),
                               static_cast<int>(info.shape[1])};
        std::vector<size_t> steps{static_cast<size_t>(

            info.strides[0])};
        // FIXME: CV_8UC3
        return cv::Mat(shape, CV_8UC3, info.ptr, steps.data());
      }))
      .def_buffer([](cv::Mat &mat) -> py::buffer_info {
        return py::buffer_info(
            mat.data,              /* Pointer to buffer */
            sizeof(unsigned char), /* Size of one scalar */
            py::format_descriptor<unsigned char>::format(), /* Python
                                                         struct-style format
                                                         descriptor */
            3,                                    /* Number of dimensions */
            {mat.rows, mat.cols, mat.channels()}, /* Buffer dimensions */
            {mat.step[0], /* Strides (in bytes) for each index */
             mat.step[1], sizeof(unsigned char)});
      });
  using mat = cyy::naive_lib::opencv::mat;
  py::class_<mat>(sub_m, "Mat")
      .def(py::init<const cv::Mat &>())
      .def("MSSIM", &mat::MSSIM);
}
