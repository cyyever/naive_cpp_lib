/*!
 * \file graph.hpp
 *
 * \brief implements graph
 */

#if __has_include(<ranges>)
#include "graph.hpp"

namespace cyy::naive_lib::data_structure {

  /* std::optional<size_t> graph::get_node_index(node_type node) const { */
  /*   auto it = node_indices.find(node); */
  /*   if (it != node_indices.end()) { */
  /*     return it->second; */
  /*   } */
  /*   return {}; */
  /* } */
  std::pair<graph::node_index_map_type, graph::adjacent_matrix_type>
  graph::get_adjecent_matrix() const {
    node_index_map_type node_indices;
    adjacent_matrix_type adjacent_matrix;
    node_indices.reserve(adjacent_list.size());
    adjacent_matrix.reserve(adjacent_list.size());

    for (auto const &[node, _] : adjacent_list) {
      node_indices[node] = adjacent_matrix.size();
      adjacent_matrix.emplace_back(adjacent_list.size(), false);
    }
    for (auto const &[node, adjacent_nodes] : adjacent_list) {
      auto from_index = node_indices[node];
      for (auto const &to_node : adjacent_nodes) {
        auto to_index = node_indices[to_node];
        adjacent_matrix[from_index][to_index] = true;
      }
    }
    return {std::move(node_indices), std::move(adjacent_matrix)};
  }
  void graph::add_edge(const edge_type &edge) {
    add_directed_edge(edge);
    auto reversed_edge = edge;
    std::swap(reversed_edge.first, reversed_edge.second);
    add_directed_edge(reversed_edge);
  }
  void graph::add_directed_edge(const edge_type &edge) {
    /* auto [it, has_inserted] = */
    /*     node_indices.try_emplace(edge.first, adjacent_list.size()); */
    /* if (has_inserted) { */
    /*   adjacent_list.emplace_back(); */
    /* } */
    adjacent_list[edge.first].push_back(edge.second);
  }
} // namespace cyy::naive_lib::data_structure
#endif
