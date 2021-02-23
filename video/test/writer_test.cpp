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

#include "../src/ffmpeg_writer.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_writer") {

  auto mat_opt = cyy::naive_lib::opencv::mat::load(STR_HELPER(IN_IMAGE));
  CHECK(mat_opt);

  cyy::naive_lib::video::ffmpeg_writer writer;
  CHECK(writer.open("a.flv", "flv", 320, 240));
  CHECK(writer.write_frame(mat_opt.value().get_cv_mat()));
}
