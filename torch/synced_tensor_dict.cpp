
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <stdexcept>

#include "hardware/hardware.hpp"
#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
#include "synced_tensor_dict_fetch_thread.hpp"
#include "synced_tensor_dict_save_thread.hpp"
namespace cyy::cxx_lib::pytorch {

  synced_tensor_dict::synced_tensor_dict(const std::string &storage_dir_)
      : storage_dir(storage_dir_) {
    cyy::cxx_lib::log::set_level(spdlog::level::level_enum::warn);
    auto cpu_num = cyy::cxx_lib::hardware::cpu_num();
    saving_thread_num = cpu_num;
    fetch_thread_num = cpu_num;
    LOG_WARN("saving_thread_num and fetch_thread_num {}", cpu_num);
    if (!storage_dir.empty()) {
      if (std::filesystem::exists(storage_dir)) {
        if (!std::filesystem::is_directory(storage_dir)) {
          throw std::invalid_argument(storage_dir.string() +
                                      " is not a directory");
        }
        for (const auto &f : std::filesystem::directory_iterator(storage_dir)) {
          if (f.is_regular_file()) {
            auto key = f.path().filename().string();
            data_info[key] = data_state::IN_DISK;
            LOG_DEBUG("load key {}", key);
          }
        }
        if (data_info.empty()) {
          LOG_WARN("no key to load");
        } else {
          LOG_WARN("load {} keys", data_info.size());
        }
      } else {
        std::filesystem::create_directories(storage_dir);
      }
    }
    for (size_t i = 0; i < saving_thread_num; i++) {
      saving_threads.emplace_back(*this, i);
    }
    for (auto &t : saving_threads) {
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
      flush_all(true);
    }
    for (size_t i = 0; i < fetch_thread_num; i++) {
      fetch_request_queue.emplace_back();
    }
    for (size_t i = 0; i < saving_thread_num; i++) {
      save_request_queue.emplace_back();
    }
    for (auto &t : fetch_threads) {
      t.stop();
    }
    for (auto &t : saving_threads) {
      t.stop();
    }
    fetch_request_queue.clear();
    save_request_queue.clear();
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
      auto [result, value_opt] = prefetch(key, false);
      if (!result) {
        throw std::out_of_range(key);
      }
      if (value_opt.has_value()) {
        return value_opt.value();
      }
      LOG_WARN("wait data {}, fetch_request_queue size is {}", key,
                fetch_request_queue.size());
      new_data_cv.wait(lk);
    }
    throw std::runtime_error("should not be here");
  }
  void synced_tensor_dict::emplace(const std::string &key,
                                   const torch::Tensor &value) {
    std::unique_lock lk(data_mutex);
    data.emplace(key, value);
    data_info[key] = data_state::IN_MEMORY_NEW_DATA;
    saving_data.erase(key);
    if (data.size() > in_memory_number) {
      auto wait_threshold =
          static_cast<size_t>(in_memory_number * wait_flush_ratio);
      lk.unlock();
      flush();
      auto old_in_memory_number = in_memory_number;
      auto remain_size = save_request_queue.size();
      if (remain_size > wait_threshold) {
        LOG_WARN("wait flush remain_size is {} wait threshold is {} ",
                 remain_size, wait_threshold);
        save_request_queue.wait_for_less_size(old_in_memory_number,
                                              std::chrono::seconds(1));
      }
      lk.lock();
    }
  }
  size_t synced_tensor_dict::size() const {
    std::lock_guard lk(data_mutex);
    return data_info.size();
  }
  std::vector<std::string> synced_tensor_dict::keys() const {
    std::vector<std::string> res;
    std::lock_guard lk(data_mutex);
    res.reserve(data_info.size());
    for (auto const &[key, __] : data_info) {
      res.emplace_back(key);
    }
    return res;
  }
  std::vector<std::string> synced_tensor_dict::in_memory_keys() const {
    std::vector<std::string> res;
    std::lock_guard lk(data_mutex);
    res.reserve(data_info.size());
    for (auto const &[key, state] : data_info) {
      if (state == data_state::IN_MEMORY ||
          state == data_state::IN_MEMORY_NEW_DATA) {
        res.emplace_back(key);
      }
    }
    return res;
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
    }
  }

  bool synced_tensor_dict::contains(const std::string &key) const {
    std::lock_guard lk(data_mutex);
    return data_info.find(key) != data_info.end();
  }
  void synced_tensor_dict::set_logging(bool enable_debug) const {
    if (enable_debug) {
      cyy::cxx_lib::log::set_level(spdlog::level::level_enum::debug);
    } else {
      cyy::cxx_lib::log::set_level(spdlog::level::level_enum::warn);
    }
  }

  void synced_tensor_dict::flush(size_t flush_num) {
    auto tasks = pop_expired_data(flush_num);
    flush(tasks);
    return;
  }
  void synced_tensor_dict::flush(std::list<save_task> &tasks) {
    for (auto &task : tasks) {
      save_request_queue.emplace_back(std::move(task));
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
        auto it = data_info.find(key);
        if (it == data_info.end()) {
          throw std::runtime_error(std::string("can't find info :" + key));
        }
        if (it->second == data_state::IN_MEMORY) {
          it->second = data_state::IN_DISK;
          continue;
        }
        if (it->second != data_state::IN_MEMORY_NEW_DATA) {
          throw std::runtime_error(std::string(
              "invalid state " + std::to_string(static_cast<int>(it->second)) +
              " of key:" + key));
        }
        it->second = data_state::PRE_SAVING;
        saving_data[key] = value;
      }
      expired_data.emplace_back(save_task{key, get_tensor_file_path(key)});
    }
    return expired_data;
  }

  void synced_tensor_dict::set_storage_dir(std::string storage_dir_) {
    if (storage_dir_.empty()) {
      throw std::invalid_argument(storage_dir_ + " is not a directory");
    }
    std::lock_guard lk(data_mutex);
    storage_dir = std::move(storage_dir_);
    if (!std::filesystem::exists(storage_dir)) {
      std::filesystem::create_directories(storage_dir);
    } else {
      if (!std::filesystem::is_directory(storage_dir)) {
        throw std::invalid_argument(storage_dir.string() +
                                    " is not a directory");
      }
    }
  }

  std::string synced_tensor_dict::get_storage_dir() const {
    std::lock_guard lk(data_mutex);
    return storage_dir.string();
  }

  void synced_tensor_dict::set_wait_flush_ratio(size_t wait_flush_ratio_) {
    std::lock_guard lk(data_mutex);
    wait_flush_ratio = wait_flush_ratio_;
  }

  void synced_tensor_dict::flush_all(bool wait) {
    std::unique_lock lk(data_mutex);
    auto old_in_memory_number = in_memory_number;
    in_memory_number = 0;
    auto tasks = pop_expired_data(SIZE_MAX);
    in_memory_number = old_in_memory_number;
    lk.unlock();
    if (tasks.empty()) {
      return;
    }
    flush(tasks);
    if (!wait) {
      return;
    }

    save_request_queue.wait_for_less_size(0, std::chrono::minutes(1));
    lk.lock();
    if (saving_data.empty()) {
      return;
    }
    flush_finished_cv.wait(lk);
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
  synced_tensor_dict::prefetch(const std::string &key, bool with_lock) {
    {
      std::unique_lock lk(data_mutex, std::defer_lock);
      if (with_lock) {
        lk.lock();
      }
      auto it = data_info.find(key);
      if (it == data_info.end()) {
        return {false, {}};
      }
      if (it->second == data_state::PRE_SAVING ||
          it->second == data_state::SAVING) {
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

      if (it->second == data_state::LOADING) {
        return {true, {}};
      }
      it->second = data_state::PRE_LOAD;
    }
    auto file_path = get_tensor_file_path(key);
    // jump to queue front
    fetch_request_queue.emplace_front(fetch_task{key, file_path});
    return {true, {}};
  }

  void synced_tensor_dict::prefetch(const std::vector<std::string> &keys) {
    for (auto const &key : keys) {
      prefetch(key);
    }
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

  void synced_tensor_dict::set_saving_thread_number(size_t saving_thread_num_) {
    if (saving_thread_num_ == 0) {
      throw std::runtime_error("saving_thread_num_ is 0");
    }

    std::unique_lock lk(data_mutex);
    if (saving_thread_num_ < saving_thread_num) {
      throw std::runtime_error("can't shrink threads");
    }
    for (size_t i = saving_thread_num; i < saving_thread_num_; i++) {
      saving_threads.emplace_back(*this, i);
      saving_threads.back().start();
    }
    saving_thread_num = saving_thread_num_;
    LOG_WARN("new saving_thread_num {}", saving_thread_num);
  }

  void synced_tensor_dict::set_fetch_thread_number(size_t fetch_thread_num_) {
    if (fetch_thread_num_ == 0) {
      throw std::runtime_error("fetch_thread_num_ is 0");
    }
    std::unique_lock lk(data_mutex);
    if (fetch_thread_num_ < fetch_thread_num) {
      throw std::runtime_error("can't shrink threads");
    }
    for (size_t i = 0; i < fetch_thread_num_ - fetch_thread_num; i++) {
      fetch_threads.emplace_back(*this);
      fetch_threads.back().start();
    }
    fetch_thread_num = fetch_thread_num_;
    LOG_WARN("new fetch_thread_num {}", fetch_thread_num);
  }
  void synced_tensor_dict::set_in_memory_number(size_t in_memory_number_) {
    LOG_WARN("set in_memory_number {}", in_memory_number_);
    std::lock_guard lk(data_mutex);
    in_memory_number = in_memory_number_;
  }
  size_t synced_tensor_dict::get_in_memory_number() const {
    std::lock_guard lk(data_mutex);
    return in_memory_number;
  }
} // namespace cyy::cxx_lib::pytorch
