
#include <algorithm>
#include <filesystem>
#include <mutex>
#include <stdexcept>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
#include "synced_tensor_dict_fetch_thread.hpp"
#include "synced_tensor_dict_save_thread.hpp"
namespace cyy::cxx_lib::pytorch {

  synced_tensor_dict::synced_tensor_dict(const std::string &storage_dir_)
      : storage_dir(storage_dir_) {

    if (!storage_dir.empty()) {
      if (std::filesystem::exists(storage_dir)) {
        if (!std::filesystem::is_directory(storage_dir)) {
          throw std::invalid_argument(storage_dir.string() +
                                      " is not a directory");
        }
        for (const auto &f : std::filesystem::directory_iterator(storage_dir)) {
          auto key = f.path().c_str();
          data_info[key] = data_state::IN_DISK;
        }
      } else {
        std::filesystem::create_directories(storage_dir);
      }
    }
    for (size_t i = 0; i < save_thread_num; i++) {
      save_threads.emplace_back(*this);
    }
    for (auto &t : save_threads) {
      t.start();
    }
    for (size_t i = 0; i < fetch_thread_num; i++) {
      fetch_threads.emplace_back(*this);
    }
    for (auto &t : fetch_threads) {
      t.start();
    }

  } // namespace cyy::cxx_lib::pytorch

  synced_tensor_dict::~synced_tensor_dict() { release(); }

  void synced_tensor_dict::release() {
    if (permanent) {
      flush_all();
    }
    for (size_t i = 0; i < fetch_thread_num; i++) {
      fetch_request_queue.emplace_back();
    }
    fetch_request_queue.wake_up_all_consumers();
    for (size_t i = 0; i < save_thread_num; i++) {
      save_request_queue.emplace_back();
    }
    save_request_queue.wake_up_all_consumers();
    for (auto &t : fetch_threads) {
      t.stop();
    }
    for (auto &t : save_threads) {
      t.stop();
    }
    data.clear();
    data_info.clear();
    saving_data.clear();

    if (!permanent && !storage_dir.empty()) {
      LOG_INFO("remove {}", storage_dir.string());
      std::filesystem::remove_all(storage_dir);
    }
  }

  torch::Tensor synced_tensor_dict::get(const std::string &key) {
    while (true) {
      std::unique_lock lk(data_mutex);
      auto [result, value_opt] = prefetch(key);
      if (!result) {
        throw std::out_of_range(key);
      }
      if (value_opt.has_value()) {
        return value_opt.value();
      }

      LOG_INFO("wait data");
      new_data_cv.wait(lk);
    }
    throw std::runtime_error("should not be here");
  }
  void synced_tensor_dict::emplace(const std::string &key,
                                   const torch::Tensor &value) {
    std::unique_lock lk(data_mutex);
    data.emplace(key, value);
    data_info[key] = data_state::IN_MEMORY_NEW_DATA;
    if (data.size() > in_memory_number) {
      flush();
      if (data.size() + saving_data.size() >
          in_memory_number * wait_flush_ratio) {
        LOG_INFO("wait flush saving_data size is {} ratio is {} data is {} "
                 "in_memory_number is {}",
                 saving_data.size(), wait_flush_ratio, data.size(),
                 in_memory_number);
        less_data_cv.wait(lk);
      }
    }
  }
  size_t synced_tensor_dict::size() const {
    std::lock_guard lk(data_mutex);
    return data_info.size();
  }

  void synced_tensor_dict::erase(const std::string &key) {
    std::lock_guard lk(data_mutex);
    if (!data_info.erase(key)) {
      throw std::out_of_range(key);
    }
    data.erase(key);
    saving_data.erase(key);
    if (!storage_dir.empty() && std::filesystem::exists(storage_dir)) {
      std::filesystem::remove(get_tensor_file_path(key));
    }
  }

  void synced_tensor_dict::clear() {
    std::lock_guard lk(data_mutex);
    data_info.clear();
    data.clear();
    saving_data.clear();
    if (!storage_dir.empty() && std::filesystem::exists(storage_dir)) {
      std::filesystem::remove_all(storage_dir);
      set_storage_dir(storage_dir);
    }
  }

  bool synced_tensor_dict::contains(const std::string &key) const {
    std::lock_guard lk(data_mutex);
    return data_info.find(key) != data_info.end();
  }
  void synced_tensor_dict::enable_debug_logging(bool enable) const {
    if (enable) {
      cyy::cxx_lib::log::set_level(spdlog::level::level_enum::debug);
    } else {
      cyy::cxx_lib::log::set_level(spdlog::level::level_enum::err);
    }
  }

  void synced_tensor_dict::flush() {
    auto tasks = pop_expired_data(SIZE_MAX);
    flush(tasks);
  }
  void synced_tensor_dict::flush(const std::list<save_task> &tasks) {
    for (auto &task : tasks) {
      save_request_queue.emplace_back(std::move(task));
    }
    if (!tasks.empty()) {
      save_request_queue.wake_up_all_consumers();
    }
  }

  std::list<synced_tensor_dict::save_task>
  synced_tensor_dict::pop_expired_data(size_t max_number) {
    std::list<save_task> expired_data;
    while (expired_data.size() < max_number) {
      std::string key;
      torch::Tensor value;
      {
        std::unique_lock lk(data_mutex);

        if (data.size() <= in_memory_number) {
          break;
        }
        std::tie(key, value) = data.pop_front();
        data_info[key] = data_state::SAVING;
        saving_data[key] = value;
      }
      expired_data.emplace_back(
          save_task{key, std::move(value), get_tensor_file_path(key)});
    }
    return expired_data;
  }

  void synced_tensor_dict::set_storage_dir(const std::string &storage_dir_) {
    std::lock_guard lk(data_mutex);
    storage_dir = storage_dir_;
    if (!std::filesystem::exists(storage_dir)) {
      std::filesystem::create_directories(storage_dir);
    }
  }

  void synced_tensor_dict::set_wait_flush_ratio(size_t wait_flush_ratio_) {
    std::lock_guard lk(data_mutex);
    wait_flush_ratio = wait_flush_ratio_;
  }

  void synced_tensor_dict::flush_all() {
    std::lock_guard lk(data_mutex);
    auto old_in_memory_number = in_memory_number;
    in_memory_number = 0;
    flush();
    in_memory_number = old_in_memory_number;
  }

  std::filesystem::path
  synced_tensor_dict::get_tensor_file_path(const std::string &key) const {
    std::lock_guard lk(data_mutex);
    if (storage_dir.empty()) {
      throw std::runtime_error("storage_dir is empty");
    }
    return storage_dir / std::filesystem::path(key);
  }

  std::pair<bool, std::optional<torch::Tensor>>
  synced_tensor_dict::prefetch(const std::string &key) {
    {
      std::lock_guard lk(data_mutex);
      auto it = data_info.find(key);
      if (it == data_info.end()) {
        return {false, {}};
      }
      if (it->second == data_state::SAVING) {
        auto node = saving_data.extract(key);
        data.emplace(key, node.mapped());
        it->second = data_state::IN_MEMORY_NEW_DATA;
        return {true, std::move(node.mapped())};
      }
      if (it->second == data_state::IN_MEMORY ||
          it->second == data_state::IN_MEMORY_NEW_DATA) {
        return {true, *data.find(key)};
      }
      if (it->second == data_state::LOAD_FAILED) {
        return {false, {}};
      }

      if (it->second != data_state::IN_DISK) {
        return {true, {}};
      }
      it->second = data_state::PRE_LOAD;
    }
    auto file_path = get_tensor_file_path(key);
    fetch_request_queue.emplace_back(fetch_task{key, file_path});
    fetch_request_queue.wake_up_all_consumers();
    return {true, {}};
  }

  void synced_tensor_dict::prefetch(const std::vector<std::string> &keys) {
    for (const auto &key : keys) {
      prefetch(key);
    }
  }
  bool synced_tensor_dict::need_flush() const {
    return data.size() > in_memory_number;
  }

  bool synced_tensor_dict::change_state(const std::string &key,
                                        data_state old_state,
                                        data_state new_state) {
    auto it = data_info.find(key);
    if (it == data_info.end()) {
      return false;
    }
    if (it->second != old_state) {
      return false;
    }
    it->second = new_state;
    return true;
  }

} // namespace cyy::cxx_lib::pytorch
