/*!
 * \file ffmpeg_writer_test.cpp
 *
 * \brief 测试ffmpeg封装
 * \author cyy
 */

#include <cstring>
#include <iostream>
#include <unistd.h>

#include <deepir/image/image.hpp>
#include <deepir/video/ffmpeg_writer.hpp>
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << " Usage: \r\n       " << argv[0]
              << " /path/to/input.jpg /path/to/saved.flv" << std::endl;
    std::cerr << "\r\nSample: \r\n       " << argv[0]
              << " /tmp/1.jpg /tmp/saved.flv" << std::endl;
    std::cerr << "       vlc /tmp/saved.flv  # display output .flv"
              << std::endl;
    return -1;
  }

  auto mat_opt = deepir::image::load(argv[1]);
  if (!mat_opt) {
    std::cerr << "load image failed" << std::endl;
    return -1;
  }

  deepir::video::ffmpeg::writer writer;
  const char *out_url = argv[2];
  if (!writer.open(out_url, "flv", 320, 240)) {
    std::cerr << "open " << out_url << " failed" << std::endl;
    return -1;
  }

  for (size_t i = 0; i < 100; i++) {

    if (!writer.write_frame(mat_opt.value())) {
      std::cerr << "write_frame failed" << std::endl;
      return -1;
    }
  }

  return 0;
}
