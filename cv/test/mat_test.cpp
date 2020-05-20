/*!
 * \file mat_test.cpp
 *
 * \brief 测试mat相关函数
 * \author cyy
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "cv/mat.hpp"

#define STR_H(x) #x
#define STR_HELPER(x) STR_H(x)

TEST_CASE("mat") {
  {
    auto tmp_mat = cyy::cxx_lib::math::mat::load(STR_HELPER(IN_IMAGE));
    CHECK(tmp_mat);
    cyy::cxx_lib::math::mat image_mat = tmp_mat.value();

#ifdef USE_GPU
    image_mat.use_gpu(true);
#else
    image_mat.use_gpu(false);
#endif

    CHECK_EQ(image_mat.channels(), 3);

    SUBCASE("clone") {
      auto image_mat2 = image_mat.clone();
      CHECK_EQ(image_mat.width(), image_mat2.width());
      CHECK_EQ(image_mat.height(), image_mat2.height());

      CHECK_NE(image_mat.get_cv_mat().data, image_mat2.get_cv_mat().data);
    }

    SUBCASE("plus") {
      auto image_mat2 = image_mat.clone();
      image_mat2 += 2;
      auto channels = image_mat.split();
      auto channels2 = image_mat2.split();
      for (size_t c = 0; c < 3; c++) {
        for (size_t i = 0; i < image_mat.get_cv_mat().total(); i++) {
          CHECK_EQ(std::min(static_cast<int>(
                                channels[c].get_cv_mat().at<uchar>(i) + 2),
                            255),
                   channels2[c].get_cv_mat().at<uchar>(i));
        }
      }
    }

    SUBCASE("plus cv::scalar") {
      auto image_mat2 = image_mat.clone();
      image_mat2 += cv::Scalar(0, 1, 2);
      auto channels = image_mat.split();
      auto channels2 = image_mat2.split();
      for (size_t c = 0; c < 3; c++) {
        for (size_t i = 0; i < image_mat.get_cv_mat().total(); i++) {
          CHECK_EQ(std::min(static_cast<int>(
                                channels[c].get_cv_mat().at<uchar>(i) + c % 3),
                            255),
                   channels2[c].get_cv_mat().at<uchar>(i));
        }
      }
    }

    SUBCASE("div") {
      auto image_mat2 = image_mat.clone();
      image_mat2 /= 2;

      auto channels = image_mat.split();
      auto channels2 = image_mat2.split();
      for (size_t c = 0; c < 3; c++) {
        for (size_t i = 0; i < image_mat.get_cv_mat().total(); i++) {
          CHECK_EQ(
              (double)channels[c].get_cv_mat().at<uchar>(i) / 2,
              doctest::Approx((double)channels2[c].get_cv_mat().at<uchar>(i))
                  .epsilon(0.5));
        }
      }
    }

    SUBCASE("convert_to") {
      auto my_float_mat = image_mat.convert_to(CV_32FC3);
      CHECK_NE(my_float_mat.type(), image_mat.type());
      CHECK(my_float_mat.elem_size() == 12);

      auto channels = image_mat.split();
      auto my_float_channels = my_float_mat.split();

      for (size_t c = 0; c < 3; c++) {
        for (size_t i = 0; i < image_mat.get_cv_mat().total(); i++) {
          CHECK_EQ(channels[c].get_cv_mat().at<uchar>(i),
                   my_float_channels[c].get_cv_mat().at<float>(i));
        }
      }
    }

    SUBCASE("cvt_color") {
      auto new_image_mat = image_mat.clone();
      new_image_mat.cvt_color(cv::COLOR_BGR2RGB);
      CHECK_EQ(image_mat.elem_size(), new_image_mat.elem_size());
      CHECK_EQ(image_mat.channels(), new_image_mat.channels());
      CHECK_EQ(image_mat.width(), new_image_mat.width());
      CHECK_EQ(image_mat.height(), new_image_mat.height());

      auto channels = image_mat.split();
      auto new_channels = new_image_mat.split();

      CHECK_EQ(channels.at(0), new_channels.at(2));
      CHECK_EQ(channels.at(1), new_channels.at(1));
      CHECK_EQ(channels.at(2), new_channels.at(0));
    }

    SUBCASE("elem_size") { CHECK(image_mat.elem_size() > 0); }

    SUBCASE("transpose") {
      auto image_mat2 = image_mat.transpose().transpose();
      CHECK(image_mat == image_mat2);
    }
    SUBCASE("split") { CHECK(image_mat.split().size() == 3); }
    SUBCASE("resize") {
      auto image_mat2 = image_mat.resize(10, 20);
      CHECK_EQ(image_mat2.width(), 10);
      CHECK_EQ(image_mat2.height(), 20);
    }
  }
}
