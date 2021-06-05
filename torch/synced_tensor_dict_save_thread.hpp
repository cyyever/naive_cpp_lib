#pragma once
#include <chrono>
#include <filesystem>
#include <mutex>
#include <stdexcept>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
#include "util/time.hpp"
namespace cyy::naive_lib::pytorch {

  class synced_tensor_dict::save_thread final
      : public cyy::naive_lib::runnable {
  public:
    explicit save_thread(synced_tensor_dict &dict_, size_t id_)
        : dict(dict_), id(id_) {}

  private:
    void run() override {
      LOG_DEBUG("run save_thread id {}", id);
      std::optional<std::optional<synced_tensor_dict::save_task>> value_opt;
      while (!needs_stop()) {
        if (id == 0) {
          value_opt =
              dict.save_request_queue.pop_front(std::chrono::milliseconds(500));
        } else {
          value_opt =
              dict.save_request_queue.pop_front(std::chrono::minutes(1));
        }
        if (!value_opt.has_value()) {
          if (id == 0) {
            dict.flush();
            LOG_DEBUG("flush by save thread");
          }
          continue;
        }
        if (!(*value_opt).has_value()) {
          return;
        }
        auto &[key, path] = value_opt.value().value();
        try {
          std::unique_lock lk(dict.data_mutex);
          if (!dict.change_state(key, data_state::PRE_SAVING,
                                 data_state::SAVING)) {
            continue;
          }
          auto value = dict.saving_data[key];
          lk.unlock();
          std::filesystem::remove(path);
          torch::save(value, path.string());
          lk.lock();
          if (dict.change_state(key, data_state::SAVING, data_state::IN_DISK)) {
            dict.saving_data.erase(key);
            LOG_DEBUG("torch::save {} succ", path.string());
            if (dict.saving_data.empty()) {
              lk.unlock();
              dict.flush_finished_cv.notify_all();
            }
            continue;
          }
          if (!dict.data_info.count(key)) {
            std::filesystem::remove(path);
          }
        } catch (const std::exception &e) {
          LOG_ERROR("torch::save {} failed,drop it:{}", path.string(),
                    e.what());
          dict.erase(key);
        }
      }
    }

  private:
    synced_tensor_dict &dict;
    size_t id;
  };
} // namespace cyy::naive_lib::pytorch
