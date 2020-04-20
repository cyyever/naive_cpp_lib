/*!
 * \file mat_test.cpp
 *
 * \brief 测试mat相关函数
 * \author cyy
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <deepir/image/image.hpp>
#include <doctest.h>
#ifdef USE_GPU
#include <deepir/allocator/buddy_pool.hpp>
#endif
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "../src/mat.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("mat") {
  {
    deepir::allocator::buddy_pool::set_device_pool_size(0, 28);
    deepir::allocator::buddy_pool::set_host_pool_size(28);

    auto tmp_mat = deepir::image::load(STR_HELPER(IN_IMAGE));
    CHECK(tmp_mat);
    deepir::math::mat image_mat = tmp_mat.value();

    image_mat.use_gpu(true);

    CHECK_EQ(image_mat.width(), tmp_mat->cols);
    CHECK_EQ(image_mat.height(), tmp_mat->rows);
    CHECK_EQ(image_mat.channels(), 3);

    SUBCASE("resize") {
      std::vector<std::thread> thds;

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

      for (auto &thd : thds) {
        thd.join();
      }
    }
  }
}
