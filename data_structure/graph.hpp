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
    using node_type = size_t;
    using edge_type = std::pair<node_type, node_type>;
    using node_index_map_type = std::unordered_map<node_type, size_t>;
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
    /* std::optional<size_t> get_node_index(node_type node) const; */
    virtual void add_edge(const edge_type &edge);
    auto const &get_adjecent_list() const { return adjacent_list; }
    std::pair<node_index_map_type, adjacent_matrix_type>
    get_adjecent_matrix() const;

  protected:
    void add_directed_edge(const edge_type &edge);

  private:
    /* std::unordered_map<node_type, size_t> node_indices; */
    std::unordered_map<node_type, std::list<node_type>> adjacent_list;
  };
  class directed_graph : public graph {
  public:
    using graph::graph;
    void add_edge(const edge_type &edge) override { add_directed_edge(edge); }
  };
} // namespace cyy::naive_lib::data_structure
#endif
