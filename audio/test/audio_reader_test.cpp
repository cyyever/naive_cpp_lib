/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <iostream>

#include <doctest/doctest.h>

#include "../src/ffmpeg_audio_reader.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_reader") {
  cyy::naive_lib::audio::ffmpeg::reader reader;
  REQUIRE(reader.open(STR_HELPER(IN_AUDIO)));

  auto duration_opt = reader.get_duration();
  CHECK(duration_opt);

  std::cout << duration_opt.value().count() << std::endl;
}
