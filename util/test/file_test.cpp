/*!
 * \file file_test.cpp
 *
 * \brief 测试file函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#ifdef _GNU_SOURCE
#include <fcntl.h>
#endif
#include "../file.hpp"

TEST_CASE("get_file_content") {
  CHECK(::cyy::naive_lib::io::get_file_content(__FILE__));
}

TEST_CASE("write_and_read") {
  int pipefd[2];
#ifdef WIN32
  CHECK_EQ(_pipe(pipefd, 10, 0), 0);
#else
#ifdef _GNU_SOURCE
  CHECK_EQ(pipe2(pipefd, O_CLOEXEC), 0);
#else
  CHECK_EQ(pipe(pipefd), 0);
#endif
#endif

  auto write_cnt_opt = ::cyy::naive_lib::io::write(
      pipefd[1], static_cast<const void *>("ab"), 2);
  CHECK(write_cnt_opt);

  CHECK_EQ(write_cnt_opt, 2);

  auto tmp = ::cyy::naive_lib::io::read(pipefd[0], 1);
  auto res = tmp.first;
  auto data = tmp.second;
  CHECK(res);
  CHECK_EQ(data.size(), 1);
  CHECK_EQ(static_cast<char>(data[0]), 'a');

  std::tie(res, data) = ::cyy::naive_lib::io::read(pipefd[0], 1);
  CHECK(res);
  CHECK_EQ(data.size(), 1);
  CHECK_EQ(static_cast<char>(data[0]), 'b');
}
