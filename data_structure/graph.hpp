/*!
 * \file graph.hpp
 *
 * \brief implements graph
 */

#pragma once

#if __has_include(<ranges>)
#include <algorithm>
#include <list>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>

namespace cyy::naive_lib::data_structure {

  template <typename vertex_type = size_t> class graph {
  public:
    using edge_type = std::pair<vertex_type, vertex_type>;
    using vertex_index_map_type = std::unordered_map<vertex_type, size_t>;
    using adjacent_matrix_type = std::vector<std::vector<bool>>;
    graph() = default;
    virtual ~graph() = default;
    template <std::ranges::input_range U>
    requires std::same_as<edge_type, std::ranges::range_value_t<U>>
    explicit graph(U edges) {
      for (auto const &edge : edges) {
        add_edge(edge);
      }
    }
    virtual void add_edge(const edge_type &edge) {
      add_directed_edge(edge);
      auto reversed_edge = edge;
      std::swap(reversed_edge.first, reversed_edge.second);
      add_directed_edge(reversed_edge);
    }
    virtual void remove_edge(const edge_type &edge) {
      remove_directed_edge(edge);
      auto reversed_edge = edge;
      std::swap(reversed_edge.first, reversed_edge.second);
      remove_directed_edge(reversed_edge);
    }
    auto const &get_adjacent_list() const { return adjacent_list; }
    auto const &get_adjacent_list(const vertex_type &vertex) const {
      return adjacent_list.at(vertex);
    }
    std::pair<vertex_index_map_type, adjacent_matrix_type>
    get_adjacent_matrix() const {
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

  protected:
    void add_directed_edge(const edge_type &edge) {
      adjacent_list[edge.first].push_back(edge.second);
    }
    void remove_directed_edge(const edge_type &edge) {
      auto it = adjacent_list.find(edge.first);
      if (it == adjacent_list.end()) {
        return;
      }
      auto &vertices = it->second;
      const auto [first, last] = std::ranges::remove(vertices, edge.second);
      vertices.erase(first, last);
    }

  protected:
    std::unordered_map<vertex_type, std::list<vertex_type>> adjacent_list;
  };
  template <typename vertex_type = size_t>
  class directed_graph : public graph<vertex_type> {
  public:
    using graph<vertex_type>::graph;
    using edge_type = graph<vertex_type>::edge_type;
    void add_edge(const edge_type &edge) override { add_directed_edge(edge); }
    void remove_edge(const edge_type &edge) override {
      remove_directed_edge(edge);
    }
    directed_graph get_transpose() const {
      directed_graph transpose;
      for (auto &[from_vertex, to_vertices] : this->adjacent_list) {
        for (auto &to_vertex : to_vertices) {
          transpose.add_edge({to_vertex, from_vertex});
        }
      }
      return transpose;
    }
  };
} // namespace cyy::naive_lib::data_structure
#endif
