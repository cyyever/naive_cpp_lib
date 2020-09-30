/*!
 * \file reader_test.cpp
 *
 * \brief 测试reader函数
 */

#include "../src/ffmpeg_reader.hpp"
#include <fstream>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    return 0;
  }
  cyy::cxx_lib::video::ffmpeg::reader reader;
  try {
    std::ofstream of("test.mp4", std::ios::binary | std::ios::trunc);
    of.write(reinterpret_cast<const char *>(Data), Size);
    reader.open("test.mp4");
    reader.get_frame_rate();
    reader.next_frame();
  } catch (...) {
  }
  return 0; // Non-zero return values are reserved for future use.
}
