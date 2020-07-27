/*!
 * \file thread_safe_container.hpp
 *
 * \brief 實現線程安全的容器
 * \author cyy
 * \date 2018-01-24
 */

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <forward_list>
#include <list>
#include <optional>
#include <shared_mutex>

namespace cyy::cxx_lib {

  //! \brief thread_safe_container 線程安全的容器模板
  template <typename ContainerType> class thread_safe_container {
  public:
    using value_type = typename ContainerType::value_type;
    using mutex_type = std::shared_mutex;
    using container_type = ContainerType;

    thread_safe_container() = default;
    ~thread_safe_container() = default;

    thread_safe_container(const thread_safe_container &) = delete;
    thread_safe_container &operator=(const thread_safe_container &) = delete;

    thread_safe_container(thread_safe_container &&) noexcept = delete;
    thread_safe_container &
    operator=(thread_safe_container &&) noexcept = delete;

  public:
    class const_reference final {
    public:
      const_reference(const container_type &container_, mutex_type &mutex_)
          : container{container_}, mutex{mutex_} {
        mutex.lock_shared();
      }

      const_reference(const const_reference &) = delete;
      const_reference &operator=(const const_reference &) = delete;

      const_reference(const_reference &&) noexcept = delete;
      const_reference &operator=(const_reference &&) noexcept = delete;

      ~const_reference() { mutex.unlock(); }
      explicit operator const container_type &() const { return container; }
      const container_type *operator->() const { return &container; }

    private:
      const container_type &container;
      mutex_type &mutex;
    };

    const_reference const_ref() const { return {container, container_mutex}; }
    bool empty() const { return const_ref()->empty(); }
    size_t size() const { return const_ref()->size(); }

  protected:
    container_type container;
    mutable mutex_type container_mutex;
  };

  //! \brief thread_safe_linear_container 線程安全的線性容器模板
  template <typename ContainerType>
  class thread_safe_linear_container
      : public thread_safe_container<ContainerType> {
  public:
    using typename thread_safe_container<ContainerType>::container_type;
    using typename thread_safe_container<ContainerType>::mutex_type;
    using typename thread_safe_container<ContainerType>::value_type;
    using thread_safe_container<ContainerType>::container;
    using thread_safe_container<ContainerType>::container_mutex;

    explicit thread_safe_linear_container(size_t max_size_ = 0)
        : max_size{max_size_} {}
    ~thread_safe_linear_container() {}

    template <typename Rep, typename Period>
    std::optional<value_type>
    back(const std::chrono::duration<Rep, Period> &rel_time) const {
      std::unique_lock<mutex_type> lock(container_mutex);
      if (wait_for_consumer_condition(
              lock, rel_time, [this]() { return !container.empty(); })) {
        return {container.back()};
      }
      return {};
    }

    template <typename Rep, typename Period>
    std::optional<value_type>
    front(const std::chrono::duration<Rep, Period> &rel_time) const {
      std::unique_lock<mutex_type> lock(container_mutex);
      if (wait_for_consumer_condition(
              lock, rel_time, [this]() { return !container.empty(); })) {
        return {container.front()};
      }
      return {};
    }

    template <typename T> void push_back(T &&value) {
      {
        std::lock_guard lock(container_mutex);
        container.push_back(std::forward<T>(value));
        if (max_size != 0) {
          while (container.size() > max_size) {
            pop_front_wrapper();
          }
        }
      }
      new_element_cv.notify_one();
    }

    template <typename... Args> void emplace_back(Args &&... args) {
      {
        std::lock_guard lock(container_mutex);
        container.emplace_back(std::forward<Args>(args)...);
        if (max_size != 0) {
          while (container.size() > max_size) {
            pop_front_wrapper();
          }
        }
      }
      new_element_cv.notify_one();
    }

    void pop_front() {
      std::unique_lock lock(container_mutex);
      if (!container.empty()) {
        pop_front_wrapper();
        lock.unlock();
        less_element_cv.notify_all();
      }
    }

    template <typename Rep, typename Period,
              typename Predicate = bool (*)(const container_type &)>
    std::optional<value_type> pop_front(
        const std::chrono::duration<Rep, Period> &rel_time,
        Predicate pred = [](const auto & /*unused*/) { return true; }) {
      std::unique_lock<mutex_type> lock(container_mutex);
      if (wait_for_consumer_condition(lock, rel_time, [this, &pred]() {
            return !container.empty() && pred(container);
          })) {
        std::optional<value_type> value{std::move(container.front())};
        pop_front_wrapper();
        lock.unlock();
        less_element_cv.notify_all();
        return value;
      }
      return {};
    }

    void clear() {
      {
        std::lock_guard lock(container_mutex);
        container.clear();
      }
      less_element_cv.notify_all();
    }

    template <typename Rep, typename Period>
    bool wait_for_less_size(
        typename container_type::size_type want_size,
        const std::chrono::duration<Rep, Period> &rel_time) const {
      std::unique_lock<mutex_type> lock(container_mutex);
      return wait_for_producer_condition(lock, rel_time, [this, want_size]() {
        return container.size() <= want_size;
      });
    }

  private:
    template <typename Rep, typename Period, typename Predicate>
    bool wait_for_consumer_condition(
        std::unique_lock<mutex_type> &lock,
        const std::chrono::duration<Rep, Period> &rel_time,
        Predicate pred) const {
      auto res = new_element_cv.wait_for(lock, rel_time,
                                         [this, &pred]() { return pred(); });
      return res;
    }

    template <typename Rep, typename Period, typename Predicate>
    bool wait_for_producer_condition(
        std::unique_lock<mutex_type> &lock,
        const std::chrono::duration<Rep, Period> &rel_time,
        Predicate pred) const {
      return less_element_cv.wait_for(lock, rel_time,
                                      [this, &pred]() { return pred(); });
    }

    void pop_front_wrapper() {
      if constexpr (std::is_same_v<std::list<value_type>, container_type> ||
                    std::is_same_v<std::deque<value_type>, container_type> ||
                    std::is_same_v<std::forward_list<value_type>,
                                   container_type>) {
        container.pop_front();
      } else {
        container.erase(container.begin());
      }
    }

  private:
    size_t max_size{0};
    mutable std::condition_variable_any new_element_cv;
    mutable std::condition_variable_any less_element_cv;
  };

  //! \brief thread_safe_linear_container 線程安全的map容器模板
  template <typename ContainerType>
  class thread_safe_map_container
      : public thread_safe_container<ContainerType> {
  public:
    using key_type = typename ContainerType::key_type;
    using mapped_type = typename ContainerType::mapped_type;
    using typename thread_safe_container<ContainerType>::mutex_type;
    using thread_safe_container<ContainerType>::container;
    using thread_safe_container<ContainerType>::container_mutex;

    template <typename CallBackType>
    void modify_value(const key_type &key, CallBackType cb) {
      std::unique_lock lock(container_mutex);
      cb(container[key]);
    }
  };
} // namespace cyy::cxx_lib
