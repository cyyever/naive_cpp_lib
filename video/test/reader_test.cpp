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
  auto [res, _] = reader.next_frame();
  CHECK(res > 0);
  res = reader.next_frame().first;
  CHECK(res == 0);
}
