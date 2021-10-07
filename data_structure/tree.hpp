/*!
 * \file tree.hpp
 *
 * \brief implements tree
 */

#pragma once

#if __has_include(<ranges>)
#include <list>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>

#include "graph.hpp"

namespace cyy::naive_lib::data_structure {

  template <typename vertex_type = size_t>
  class tree : public graph<vertex_type> {
  public:
    using graph<vertex_type>::graph;
    using edge_type = graph<vertex_type>::edge_type;

    template <std::ranges::input_range U>
    requires std::same_as<edge_type, std::ranges::range_value_t<U>>
    explicit tree(U edges, std::optional<vertex_type> root_)
        : graph<vertex_type>(edges), root(std::move(root_)) {
      // TODO check connectivity
    }

  private:
    std::optional<vertex_type> root;
  };
} // namespace cyy::naive_lib::data_structure
#endif
