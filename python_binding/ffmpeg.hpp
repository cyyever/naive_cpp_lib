/*!
 * \file ffmpeg_read.hpp
 *
 * \author cyy
 */

#pragma once
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "video/src/ffmpeg_video_reader.hpp"
#include "video/src/ffmpeg_writer.hpp"
namespace py = pybind11;
inline void define_video_extension(py::module_ &m) {
  py::class_<cv::Mat>(m, "Matrix", py::buffer_protocol())
      .def(py::init([](py::buffer mat) {
        /* Request a buffer descriptor from Python */
        py::buffer_info info = mat.request();
        std::vector<int> shape;
        for (auto s : info.shape) {
          shape.push_back(s);
        }
        std::vector<size_t> steps;
        for (auto s : info.strides) {
          steps.push_back(s);
        }
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

  auto sub_m = m.def_submodule("video", "Contains video decoding");
  using frame = cyy::naive_lib::video::frame;
  py::class_<frame>(sub_m, "Frame")
      .def_readwrite("seq", &frame::seq)
      .def_readwrite("content", &frame::content)
      .def_readwrite("is_key", &frame::is_key);

  using ffmpeg_video_reader = cyy::naive_lib::video::ffmpeg_reader;

  py::class_<ffmpeg_video_reader>(sub_m, "FFmpegVideoReader",
                                  py::buffer_protocol())
      .def(py::init<>())
      .def("open", &ffmpeg_video_reader::open)
      .def("close", &ffmpeg_video_reader::close)
      .def("set_play_frame_rate", &ffmpeg_video_reader::set_play_frame_rate)
      .def("get_video_height", &ffmpeg_video_reader::get_video_height)
      .def("get_video_width", &ffmpeg_video_reader::get_video_width)
      .def("get_frame_rate", &ffmpeg_video_reader::get_frame_rate)
      .def("next_frame", &ffmpeg_video_reader::next_frame)
      .def("drop_non_key_frames", &ffmpeg_video_reader::drop_non_key_frames)
      .def("keep_non_key_frames", &ffmpeg_video_reader::keep_non_key_frames)
      .def("seek_frame", &ffmpeg_video_reader::seek_frame);
  using ffmpeg_video_writer = cyy::naive_lib::video::ffmpeg_writer;

  py::class_<ffmpeg_video_writer>(sub_m, "FFmpegVideoWriter",
                                  py::buffer_protocol())
      .def(py::init<>())
      .def("open", &ffmpeg_video_writer::open)
      .def("close", &ffmpeg_video_writer::close)
      .def("write_frame", &ffmpeg_video_writer::write_frame);
}
