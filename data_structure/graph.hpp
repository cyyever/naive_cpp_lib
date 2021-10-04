/*!
 * \file graph.hpp
 *
 * \brief implements graph
 */

#pragma once

#if __has_include(<ranges>)
#include <list>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cyy::naive_lib::data_structure {

  class graph {
  public:
    using vertex_type = size_t;
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
    virtual void add_edge(const edge_type &edge);
    virtual void remove_edge(const edge_type &edge);
    auto const &get_adjecent_list() const { return adjacent_list; }
    auto const &get_adjecent_list(const vertex_type &vertex) const {
      return adjacent_list.at(vertex);
    }
    std::pair<vertex_index_map_type, adjacent_matrix_type>
    get_adjecent_matrix() const;

  protected:
    void add_directed_edge(const edge_type &edge);
    void remove_directed_edge(const edge_type &edge);

  protected:
    std::unordered_map<vertex_type, std::list<vertex_type>> adjacent_list;
  };
  class directed_graph : public graph {
  public:
    using graph::graph;
    void add_edge(const edge_type &edge) override { add_directed_edge(edge); }
    void remove_edge(const edge_type &edge) override {
      remove_directed_edge(edge);
    }
    directed_graph get_transpose() const;
  };
} // namespace cyy::naive_lib::data_structure
#endif
