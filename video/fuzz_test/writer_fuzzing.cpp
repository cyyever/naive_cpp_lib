/*!
 * \file writer_test.cpp
 *
 * \brief 测试writer函数
 */

#include <fstream>
#include <iostream>

#include "../ffmpeg_video_writer.hpp"
#include "cv/mat.hpp"
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }
  try {
    auto mat_opt = cyy::naive_lib::opencv::mat::load(Data, Size);
    if (!mat_opt) {
      return 0;
    }

    cyy::naive_lib::video::ffmpeg_writer writer;
    writer.open("test.flv", "flv", 320, 240);
    writer.write_frame(mat_opt->get_cv_mat());
  } catch (...) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
