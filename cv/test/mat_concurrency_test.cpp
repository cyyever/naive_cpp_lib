/*!
 * \file mat_test.cpp
 *
 * \brief 测试mat相关函数
 * \author cyy
 */
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include <doctest/doctest.h>

#include "cv/mat.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("mat") {
  {

    auto tmp_mat = cyy::naive_lib::opencv::mat::load(STR_HELPER(IN_IMAGE));
    CHECK(tmp_mat);
    auto image_mat = tmp_mat.value();

    image_mat.use_gpu(true);

    CHECK_EQ(image_mat.channels(), 3);

    SUBCASE("resize") {
      std::vector<std::jthread> thds;

      std::mutex mu;
      for (int i = 0; i < 8; i++) {
        thds.emplace_back([&mu, &image_mat]() {
          auto image_mat2 = image_mat.resize(10, 20);
          {
            std::lock_guard lk(mu);

            CHECK_EQ(image_mat2.width(), 10);
            CHECK_EQ(image_mat2.height(), 20);
          }
        });
      }
    }
  }
}
