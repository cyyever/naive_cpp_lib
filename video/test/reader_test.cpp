/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <iostream>

#include <doctest/doctest.h>

#include "../src/ffmpeg_video_reader.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_reader") {
  cyy::naive_lib::video::ffmpeg_reader reader;
  CHECK(reader.open(STR_HELPER(IN_URL)));
  CHECK(reader.get_frame_rate());
  auto [res, frame] = reader.next_frame();
  CHECK(res > 0);
  CHECK(frame.seq == 1);
  CHECK(frame.is_key);
  std::tie(res, frame) = reader.next_frame();
  CHECK(res == 0);
  CHECK(reader.seek_frame(1));
}
