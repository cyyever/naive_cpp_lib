#pragma once
#include <filesystem>
#include <mutex>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>

#include <torch/torch.h>

#include "util/ordered_dict.hpp"
#include "util/runnable.hpp"
#include "util/thread_safe_container.hpp"

namespace cyy::cxx_lib::pytorch {
  class synced_tensor_dict final {
  public:
    explicit synced_tensor_dict(const std::string &storage_dir_);

    synced_tensor_dict(const synced_tensor_dict &) = delete;
    synced_tensor_dict &operator=(const synced_tensor_dict &) = delete;

    synced_tensor_dict(synced_tensor_dict &&) noexcept = delete;
    synced_tensor_dict &operator=(synced_tensor_dict &&) noexcept = delete;

    ~synced_tensor_dict();
    void release();
    void emplace(const std::string &key, const torch::Tensor &value);
    torch::Tensor get(const std::string &key);
    size_t size() const;
    void erase(const std::string &key);
    bool contains(const std::string &key) const;
    void enable_debug_logging(bool enable) const;
    std::vector<std::string> keys() const;
    void flush_all();
    void flush();
    void clear();
    void prefetch(const std::vector<std::string> &keys);
    void set_in_memory_number(size_t in_memory_number_) {
      std::lock_guard lk(data_mutex);
      in_memory_number = in_memory_number_;
    }
    void set_storage_dir(const std::string &storage_dir_);
    void set_wait_flush_ratio(size_t wait_flush_ratio_);
    void set_saving_thread_number(size_t saving_thread_number_);
    void set_fetch_thread_number(size_t fetch_thread_number_);

    void enable_permanent_storage() { permanent = true; }
    void disable_permanent_storage() { permanent =false; }

  private:
    enum class data_state:int {
      IN_MEMORY=0,
      IN_MEMORY_NEW_DATA,
      IN_DISK,
      PRE_SAVING,
      SAVING,
      PRE_LOAD,
      LOADING,
      LOAD_FAILED,
    };
    class save_thread;
    class fetch_thread;

    bool change_state(const std::string &key, data_state old_state,
                      data_state new_state);
    std::filesystem::path get_tensor_file_path(const std::string &key) const;
    bool need_flush() const;

    std::pair<bool, std::optional<torch::Tensor>>
    prefetch(const std::string &key);
    using save_task =
        std::tuple<std::string, std::filesystem::path>;
    std::list<save_task> pop_expired_data(size_t max_number);
    void flush(const std::list<save_task> &tasks);

    mutable std::recursive_mutex data_mutex;
    std::filesystem::path storage_dir;
    cyy::cxx_lib::ordered_dict<std::string, torch::Tensor> data;
    std::unordered_map<std::string, torch::Tensor> saving_data;
    std::unordered_map<std::string, data_state> data_info;

    using save_request_queue_type = cyy::cxx_lib::thread_safe_linear_container<
        std::list<std::optional<save_task>>>;
    save_request_queue_type save_request_queue;
    size_t saving_thread_num{2};
    std::list<save_thread> saving_threads;

    using fetch_task = std::pair<std::string, std::filesystem::path>;
    using fetch_request_queue_type = cyy::cxx_lib::thread_safe_linear_container<
        std::list<std::optional<fetch_task>>>;
    fetch_request_queue_type fetch_request_queue;
    size_t fetch_thread_num{2};
    std::list<fetch_thread> fetch_threads;

    size_t in_memory_number{128};
    bool permanent{true};
    std::condition_variable_any new_data_cv;
    std::condition_variable_any less_data_cv;
    size_t wait_flush_ratio{1};
  };
} // namespace cyy::cxx_lib::pytorch
