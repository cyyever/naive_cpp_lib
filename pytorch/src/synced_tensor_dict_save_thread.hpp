#include <cyy/cpp_lib/log/log.hpp>
#include <stdexcept>

#include "synced_tensor_dict.hpp"
namespace cyy::cxx_lib::pytorch {

  class synced_tensor_dict::save_thread final : public cyy::cxx_lib::runnable {
  public:
    save_thread(synced_tensor_dict &dict_) : dict(dict_) {}

  private:
    void run() override {
      while (true) {
        auto value_opt =
            dict.save_request_queue.pop_front(std::chrono::seconds(1));
        if (!value_opt.has_value()) {
          continue;
        }
        if (!(*value_opt).has_value()) {
          return;
        }
        auto &[key, value, path] = value_opt.value().value();
        try {
          torch::save(value, path.string());
          std::lock_guard lk(dict.data_mutex);
          if (dict.change_state(key, data_state::SAVING, data_state::IN_DISK)) {
            dict.saving_data.erase(key);
            dict.less_data_cv.notify_all();
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
