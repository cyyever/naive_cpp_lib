#include <pybind11/pybind11.h>
#ifdef BUILD_CV_PYTHON_BINDING
#include "cv.hpp"
#endif
#ifdef BUILD_VIDEO_PYTHON_BINDING
#include "cv.hpp"
#include "ffmpeg.hpp"
#endif
#ifdef BUILD_TORCH_PYTHON_BINDING
#include "stochastic_quantization.hpp"
#include "synced_tensor_dict.hpp"
#endif
namespace py = pybind11;

PYBIND11_MODULE(cyy_naive_cpp_extension, m) {
#ifdef BUILD_CV_PYTHON_BINDING
  define_cv_extension(m);
#endif
#ifdef BUILD_VIDEO_PYTHON_BINDING
  define_video_extension(m);
#endif

#ifdef BUILD_TORCH_PYTHON_BINDING
  define_torch_data_structure_extension(m);
  define_torch_quantization_extension(m);
#endif
}
