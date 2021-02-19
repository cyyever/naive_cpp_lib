#pragma once

#include <torch/csrc/api/include/torch/serialize.h>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
#include "util/runnable.hpp"
namespace cyy::naive_lib::pytorch {

  class synced_tensor_dict::fetch_thread final
      : public cyy::naive_lib::runnable {
  public:
    fetch_thread(synced_tensor_dict &dict_) : dict(dict_) {}
    ~fetch_thread() override { stop(); }

  private:
    void run() override {
      while (true) {
        auto value_opt =
            dict.fetch_request_queue.pop_front(std::chrono::minutes(1));
        if (!value_opt.has_value()) {
          continue;
        }
        if (!(*value_opt).has_value()) {
          return;
        }
        auto const &[key, path] = value_opt.value().value();
        try {
          {
            std::lock_guard lk(dict.data_mutex);
            if (!dict.change_state(key, data_state::PRE_LOAD,
                                   data_state::LOADING)) {
              continue;
            }
          }
          torch::Tensor value;
          torch::load(value, path.string());
          {
            std::lock_guard lk(dict.data_mutex);
            if (!dict.change_state(key, data_state::LOADING,
                                   data_state::IN_MEMORY)) {
              continue;
            }
            dict.data.emplace(key, std::move(value));
          }
          LOG_DEBUG("torch::load {} succ", key);
        } catch (const std::exception &e) {
          LOG_ERROR("torch::load {} failed:{}", path.string(), e.what());
          {
            std::lock_guard lk(dict.data_mutex);
            if (!dict.change_state(key, data_state::LOADING,
                                   data_state::LOAD_FAILED)) {
              continue;
            }
          }
        }
        dict.new_data_cv.notify_all();
      }
    }

  private:
    synced_tensor_dict &dict;
  };

} // namespace cyy::naive_lib::pytorch
