/*!
 * \file graph.hpp
 *
 * \brief implements graph
 */

#if __has_include(<ranges>)

#include "graph.hpp"

#include <algorithm>

namespace cyy::naive_lib::data_structure {

  std::pair<graph::vertex_index_map_type, graph::adjacent_matrix_type>
  graph::get_adjecent_matrix() const {
    vertex_index_map_type vertex_indices;
    adjacent_matrix_type adjacent_matrix;
    vertex_indices.reserve(adjacent_list.size());
    adjacent_matrix.reserve(adjacent_list.size());

    for (auto const &[vertex, _] : adjacent_list) {
      vertex_indices[vertex] = adjacent_matrix.size();
      adjacent_matrix.emplace_back(adjacent_list.size(), false);
    }
    for (auto const &[vertex, adjacent_vertices] : adjacent_list) {
      auto from_index = vertex_indices[vertex];
      for (auto const &to_vertex : adjacent_vertices) {
        auto to_index = vertex_indices[to_vertex];
        adjacent_matrix[from_index][to_index] = true;
      }
    }
    return {std::move(vertex_indices), std::move(adjacent_matrix)};
  }
  void graph::add_edge(const edge_type &edge) {
    add_directed_edge(edge);
    auto reversed_edge = edge;
    std::swap(reversed_edge.first, reversed_edge.second);
    add_directed_edge(reversed_edge);
  }
  void graph::remove_edge(const edge_type &edge) {
    remove_directed_edge(edge);
    auto reversed_edge = edge;
    std::swap(reversed_edge.first, reversed_edge.second);
    remove_directed_edge(reversed_edge);
  }
  void graph::add_directed_edge(const edge_type &edge) {
    adjacent_list[edge.first].push_back(edge.second);
  }
  void graph::remove_directed_edge(const edge_type &edge) {
    auto it = adjacent_list.find(edge.first);
    if (it == adjacent_list.end()) {
      return;
    }
    auto &vertices = it->second;
    const auto [first, last] = std::ranges::remove(vertices, edge.second);
    vertices.erase(first, last);
  }
  directed_graph directed_graph::get_transpose() const {
    directed_graph transpose;
    for (auto &[from_vertex, to_vertices] : adjacent_list) {
      for (auto &to_vertex : to_vertices) {
        transpose.add_edge({to_vertex, from_vertex});
      }
    }

    return transpose;
  }
} // namespace cyy::naive_lib::data_structure
#endif
