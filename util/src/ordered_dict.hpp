/*!
 * \file ordered_dict.hpp
 *
 * \brief implements python's OrderedDict in C++
 */

#pragma once

#include <list>
#include <unordered_map>
#include <utility>
namespace cyy::cxx_lib {

  template <class Key, class T> class ordered_dict {
  public:
    using key_type = Key;
    using mapped_type = T;
    using data_list_type = std::list<std::pair<key_type, mapped_type>>;
    using data_index_type =
        std::unordered_map<key_type, typename data_list_type::iterator>;
    struct iterator : public data_list_type::iterator {
      iterator(typename data_list_type::iterator rhs)
          : data_list_type::iterator{rhs} {}
      auto &operator*() const {
        return data_list_type::iterator::operator*().second;
      }
    };

    struct const_iterator : public data_list_type::const_iterator {
      const_iterator(typename data_list_type::const_iterator rhs)
          : data_list_type::const_iterator{rhs} {}
      const auto &operator*() const {
        return data_list_type::const_iterator::operator*().second;
      }
    };
    /* using iterator = typename data_list_type::iterator; */
    /* using const_iterator = typename data_list_type::const_iterator; */

    bool empty() const noexcept { return data.empty(); }
    auto size() const noexcept { return data.size(); }
    void clear() noexcept {
      data.clear();
      data_index.clear();
    }
    template <class... Args> bool emplace(Key &&key, Args &&... args) {
      auto [it, inserted] =
          data_index.emplace(std::forward<Key>(key), data.end());
      if (!inserted && !move_to_end_in_update) {
        (*(it->second)).second = mapped_type(std::forward<Args>(args)...);
        return false;
      }
      data.emplace_back(it->first, mapped_type(std::forward<Args>(args)...));
      auto it2 = data.end();
      it2--;
      it->second = it2;
      return inserted;
    }

    bool erase(const key_type &key) {
      auto node = data_index.extract(key);
      if (node.empty()) {
        return false;
      }
      auto const &it2 = node.mapped();
      if (it2 != data.end()) {
        data.erase(it2);
      }
      return true;
    }
    auto begin() noexcept { return iterator(data.begin()); }
    auto begin() const noexcept { return const_iterator(data.begin()); }
    auto cbegin() const noexcept { return const_iterator(data.cbegin()); }
    auto end() noexcept { return iterator(data.end()); }
    auto end() const noexcept { return const_iterator(data.end()); }
    auto cend() const noexcept { return const_iterator(data.cend()); }
    iterator find(const Key &key) {
      auto it = data_index.find(key);
      if (it == data_index.end()) {
        return data.end();
      }
      return iterator(it->second);
    }

    void move_to_end(iterator it) {
      data.emplace_back(std::move(*it));
      data.erase(it);
      it = data.end();
      it--;
      data_index[it->first] = it;
    }

  private:
    data_list_type data;
    data_index_type data_index;
    bool move_to_end_in_update{true};
  };
} // namespace cyy::cxx_lib
