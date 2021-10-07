/*!
 * \file container_test.cpp
 *
 * \brief 测试container相关函数
 * \author cyy
 */

#include <doctest/doctest.h>

#include "data_structure/graph.hpp"

TEST_CASE("graph") {
  cyy::naive_lib::data_structure::directed_graph g;
  SUBCASE("add edge") { g.add_edge({1, 2}); }
  SUBCASE("remove edge") { g.remove_edge({1, 2}); }
  SUBCASE("transpose") { g.get_transpose(); }
}
