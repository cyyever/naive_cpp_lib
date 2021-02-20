/*!
 * \file reader_test.cpp
 *
 * \brief
 */

#include <doctest/doctest.h>

#include "../src/ffmpeg_video_reader.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("ffmpeg_reader") {
  cyy::naive_lib::video::ffmpeg_reader reader;
  CHECK(reader.open(STR_HELPER(IN_URL)));
  CHECK(reader.get_frame_rate());
  reader.drop_non_key_frames();
  std::vector<cyy::naive_lib::video::frame> frames;
  for (size_t i = 0; i < 3; i++) {
    auto [res, frame] = reader.next_frame();
    CHECK(res >= 0);
    if (res == 0) {
      break;
    }
    frames.emplace_back(std::move(frame));
  }
  CHECK(frames.size() > 1);

  auto seek_res = reader.seek_frame(1);
  CHECK(seek_res);
  std::vector<cyy::naive_lib::video::frame> reread_frames;

  for (size_t i = 0; i < 3; i++) {
    auto [res, frame] = reader.next_frame();
    CHECK(res >= 0);
    if (res == 0) {
      break;
    }
    reread_frames.emplace_back(std::move(frame));
  }
  CHECK(frames == reread_frames);
}
