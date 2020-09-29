/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <iostream>

#include "../src/ffmpeg_reader.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_reader") {
  deepir::video::ffmpeg::reader reader;
  CHECK(reader.open(STR_HELPER(IN_URL)));
  CHECK(reader.get_frame_rate());
  auto [res, _] = reader.next_frame();
  CHECK(res > 0);
}