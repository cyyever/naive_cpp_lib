#include <pybind11/pybind11.h>
#ifdef  BUILD_VIDEO_PYTHON_BINDING
#include "ffmpeg_reader.hpp"
#endif
#ifdef  BUILD_TORCH_PYTHON_BINDING
#include "synced_tensor_dict.hpp"
#endif
namespace py = pybind11;

PYBIND11_MODULE(cyy_naive_cpp_extension, m) {
#ifdef  BUILD_VIDEO_PYTHON_BINDING
define_video_extension(m);
#endif
          
#ifdef  BUILD_TORCH_PYTHON_BINDING
define_torch_extension(m);
#endif
}
