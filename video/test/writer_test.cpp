/*!
 * \file string_test.cpp
 *
 * \brief 测试string函数
 * \author cyy
 * \date 2017-01-17
 */

#include <iostream>

#include <cv/mat.hpp>
#include <doctest/doctest.h>

#include "../ffmpeg_video_reader.hpp"
#include "../ffmpeg_video_writer.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_writer") {

  auto mat_opt = cyy::naive_lib::opencv::mat::load(STR_HELPER(IN_IMAGE));
  REQUIRE(mat_opt);

  cyy::naive_lib::video::ffmpeg_writer writer;
  REQUIRE(writer.open("a.h264", "h264", 320, 240));
  for (size_t i = 0; i < 250; i++) {
    REQUIRE(writer.write_frame(mat_opt.value().get_cv_mat()));
  }
  writer.close();
  cyy::naive_lib::video::ffmpeg_reader reader;
  REQUIRE(reader.open("a.h264"));
  REQUIRE(reader.get_frame_rate());
  for (size_t i = 0; i < 3; i++) {
    auto [res, frame] = reader.next_frame();
    REQUIRE(res > 0);
    REQUIRE(cyy::naive_lib::opencv::mat(frame.content).width() == 320);
  }
}
