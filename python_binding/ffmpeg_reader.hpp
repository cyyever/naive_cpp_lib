#include <pybind11/pybind11.h>

#include "video/src/ffmpeg_video_reader.hpp"
namespace py = pybind11;
inline void define_video_extension(  py::module_ &m) {
  using ffmpeg_video_reader = cyy::naive_lib::video::ffmpeg::reader;
  auto sub_m = m.def_submodule("video", "Contains video decoding");
  py::class_<ffmpeg_video_reader>(sub_m, "FFmpegVideoReader")
      .def(py::init<>())
      .def("open", &ffmpeg_video_reader::open)
      .def("close", &ffmpeg_video_reader::close)
      .def("set_play_frame_rate", &ffmpeg_video_reader::set_play_frame_rate)
      .def("get_frame_rate", &ffmpeg_video_reader::get_frame_rate)
      .def("next_frame", &ffmpeg_video_reader::next_frame)
      .def("drop_non_key_frames", &ffmpeg_video_reader::drop_non_key_frames);
}
