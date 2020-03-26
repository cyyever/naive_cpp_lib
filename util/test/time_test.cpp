/*!
 * \file time_test.cpp
 *
 * \brief 测试time函数
 * \author cyy
 * \date 2017-01-17
 */

#include <doctest/doctest.h>

#include "util/time.hpp"

TEST_CASE("now_ms") { CHECK(cyy::cxx::time::now_ms() > 0); }
