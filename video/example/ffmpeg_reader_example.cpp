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
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << " Usage: \r\n       " << argv[0] << " url" << std::endl;
    std::cerr << "\r\nSample: \r\n       # install opencv3.4 with "
                 "libgtk2.0-dev support first, then"
              << std::endl;
    std::cerr << "       " << argv[0] << " /dev/video0" << std::endl;
    std::cerr << "       " << argv[0]
              << " rtsp://admin:password@192.168.199.154:554/h264" << std::endl;
    return -1;
  }

  cyy::naive_lib::video::ffmpeg::reader reader;

  const char *url = argv[1];
  if (!reader.open(url)) {
    std::cerr << "open " << url << " failed" << std::endl;
    return -1;
  }

  reader.set_play_frame_rate({25, 1});

  auto frame_rate = reader.get_frame_rate().value();
  std::cout << "frame rate=[" << frame_rate[0] << "," << frame_rate[1] << "]"
            << std::endl;
  reader.drop_non_key_frames();

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
    std::cout << "seq is "<<frame.seq << std::endl;

    cv::Mat tmp;
    cv::resize(frame.content, tmp, cv::Size(640, 480));
    /* cv::imwrite(std::to_string(frame.seq)+".jpg",tmp); */
    cv::imshow("win", tmp);
    cv::waitKey(1);
  }
  return 0;
}
