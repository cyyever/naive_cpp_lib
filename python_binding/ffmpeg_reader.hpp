#include <pybind11/pybind11.h>

#include "video/src/ffmpeg_video_reader.hpp"
namespace py = pybind11;
inline void define_video_extension(  py::module_ &m) {

  py::class_<cv::Mat>(m, "Matrix", py::buffer_protocol())
	.def_buffer([](cv::Mat &mat) -> py::buffer_info {
		return py::buffer_info(
            mat.ptr(),/* Pointer to buffer */
            sizeof(uint8_t), /* Size of one scalar */
			py::format_descriptor<uint8_t>::format(), /* Python struct-style format descriptor */
			2,                                      /* Number of dimensions */
			{ mat.rows, mat.cols },                 /* Buffer dimensions */
			{ mat.step[0],             /* Strides (in bytes) for each index */
            mat.step[1]}
			);
		});

  auto sub_m = m.def_submodule("video", "Contains video decoding");
  using  frame= cyy::naive_lib::video::frame;
  py::class_<frame>(sub_m, "Frame")
    .def_readwrite("seq",&frame::seq)
    .def_readwrite("content",&frame::content)
    .def_readwrite("is_key",&frame::is_key);

  using ffmpeg_video_reader = cyy::naive_lib::video::ffmpeg::reader;

  py::class_<ffmpeg_video_reader>(sub_m, "FFmpegVideoReader",py::buffer_protocol())
      .def(py::init<>())
      .def("open", &ffmpeg_video_reader::open)
      .def("close", &ffmpeg_video_reader::close)
      .def("set_play_frame_rate", &ffmpeg_video_reader::set_play_frame_rate)
      .def("get_frame_rate", &ffmpeg_video_reader::get_frame_rate)
      .def("next_frame", &ffmpeg_video_reader::next_frame)
      .def("drop_non_key_frames", &ffmpeg_video_reader::drop_non_key_frames);
}
