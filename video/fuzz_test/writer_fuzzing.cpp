/*!
 * \file writer_test.cpp
 *
 * \brief 测试writer函数
 */

#include <iostream>
#include <deepir/image/image.hpp>
#include <fstream>
#include "../src/ffmpeg_writer.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if(Size==0) {
    return 0;
  }
  deepir::video::ffmpeg::writer writer;
  try{
  auto mat_opt=::deepir::image::load(Data,Size);
  if(!mat_opt) {
    return 0;
  }

  deepir::video::ffmpeg::writer writer;
  writer.open("test.flv","flv", 320, 240);
  writer.write_frame(mat_opt.value());
  }catch(...){
  }
  return 0; // Non-zero return values are reserved for future use.
}
