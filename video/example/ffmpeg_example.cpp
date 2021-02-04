/*!
 * \file ffmpeg_reader_test.cpp
 *
 * \brief 测试ffmpeg封装
 * \author cyy
 */

#include <cstring>
#include <iostream>
#include <unistd.h>

#include "../src/ffmpeg_reader.hpp"
#include "../src/ffmpeg_writer.hpp"
#include <opencv2/opencv.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << " Usage: \r\n       " << argv[0] << " in-url out-url"
              << std::endl;
    std::cerr << "\r\nSample: \r\n       " << argv[0]
              << " /dev/video0 /tmp/saved.flv" << std::endl;
    std::cerr
        << "       " << argv[0]
        << " rtsp://admin:password@192.168.199.154:554/h264 /tmp/saved.flv"
        << std::endl;
    std::cerr << "       $vlc /tmp/saved.flv  # display output .flv"
              << std::endl;
    return -1;
  }

  cyy::naive_lib::video::ffmpeg::reader reader;

  const char *in_url = argv[1];
  if (!reader.open(in_url)) {
    std::cerr << "open " << in_url << " failed" << std::endl;
    return -1;
  }

  auto frame_rate = reader.get_frame_rate().value();
  std::cout << "frame rate=[" << frame_rate[0] << "," << frame_rate[1] << "]"
            << std::endl;

  cyy::naive_lib::video::ffmpeg::writer writer;
  const char *out_url = argv[2];
  if (!writer.open(out_url, "flv", 1280, 720)) {
    std::cerr << "open " << out_url << " failed" << std::endl;
    return -1;
  }

  while (true) {
    auto [res, frame] = reader.next_frame();
    if (res < 0) {
      std::cerr << "get frame failed" << std::endl;
      return -1;
    }
    if (res == 0) {
      std::cout << "reach EOF" << std::endl;
      break;
    }

    if (!writer.write_frame(frame.content)) {
      std::cerr << "write_frame failed" << std::endl;
      return -1;
    }
  }
  return 0;
}
