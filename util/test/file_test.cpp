/*!
 * \file file_test.cpp
 *
 * \brief 测试file函数
 * \author cyy
 * \date 2017-01-17
 */

#include "../file.hpp"
#include <doctest/doctest.h>

TEST_CASE("get_file_content") {
  CHECK(::cyy::cxx_lib::io::get_file_content(__FILE__));
}

TEST_CASE("write_and_read") {
  int pipefd[2];
#ifdef WIN32
  CHECK_EQ(_pipe(pipefd, 10, 0), 0);
#else
  CHECK_EQ(pipe(pipefd), 0);
#endif

  auto write_cnt_opt =
      ::cyy::cxx_lib::io::write(pipefd[1], static_cast<const void *>("ab"), 2);
  CHECK(write_cnt_opt);

  CHECK_EQ(write_cnt_opt.value(), 2);

  auto data_opt = ::cyy::cxx_lib::io::read(pipefd[0], 1);
  CHECK(data_opt);
  CHECK_EQ(data_opt.value().size(), 1);
  CHECK_EQ((char)(data_opt.value()[0]), 'a');

  data_opt = ::cyy::cxx_lib::io::read(pipefd[0], 1);
  CHECK(data_opt);
  CHECK_EQ(data_opt.value().size(), 1);
  CHECK_EQ((char)(data_opt.value()[0]), 'b');
}
