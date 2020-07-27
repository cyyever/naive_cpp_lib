#include <filesystem>
#include <mutex>
#include <stdexcept>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
namespace cyy::cxx_lib::pytorch {

  class synced_tensor_dict::save_thread final : public cyy::cxx_lib::runnable {
  public:
    explicit save_thread(synced_tensor_dict &dict_) : dict(dict_) {}

  private:
    void run() override {
      while (true) {
        auto value_opt =
            dict.save_request_queue.pop_front(std::chrono::minutes(1));
        if (!value_opt.has_value()) {
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
            dict.less_data_cv.notify_all();
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
  };
} // namespace cyy::cxx_lib::pytorch
