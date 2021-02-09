#include <pybind11/pybind11.h>

#include "../src/ffmpeg_video_reader.hpp"
namespace py = pybind11;
PYBIND11_MODULE(cyy_naive_cpp_extension, m) {
  using ffmpeg_video_reader = cyy::naive_lib::video::ffmpeg::reader;
  auto sub_m = m.def_submodule("video", "Contains video decoding");
  py::class_<ffmpeg_video_reader>(sub_m, "FFmpegVideoReader")
      .def(py::init<>())
      .def("open", &ffmpeg_video_reader::open)
      .def("close", &ffmpeg_video_reader::close)
      .def("drop_non_key_frames", &ffmpeg_video_reader::drop_non_key_frames)
      ;
}
