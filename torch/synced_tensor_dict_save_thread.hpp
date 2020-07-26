#include <filesystem>
#include <lmdb++.h>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
namespace cyy::cxx_lib::pytorch {

  class synced_tensor_dict::save_thread final : public cyy::cxx_lib::runnable {
  public:
    explicit save_thread(synced_tensor_dict &dict_) : dict(dict_) {}

  private:
    void run() override {
      auto env = std::any_cast<std::shared_ptr<lmdb::env>>(dict.storage_handle);
      while (true) {
        auto value_opt =
            dict.save_request_queue.pop_front(std::chrono::seconds(1));
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
          std::ostringstream os;
          torch::save(value, os);
          os.flush();
          auto tensor_str = os.str();
          auto txn = lmdb::txn::begin(*env);
          auto dbi = lmdb::dbi::open(txn);
          auto res = dbi.put(txn, key, tensor_str);
          txn.commit();
          if (!res) {
            LOG_ERROR("lmdb put {} failed,drop it", key);
            dict.erase(key);
          }

          lk.lock();
          if (dict.change_state(key, data_state::SAVING, data_state::IN_DISK)) {
            dict.saving_data.erase(key);
            LOG_DEBUG("torch::save {} succ", key);
            dict.less_data_cv.notify_all();
            continue;
          }
          if (!dict.data_info.count(key)) {
            std::filesystem::remove(path);
          }
        } catch (const std::exception &e) {
          LOG_ERROR("torch::save {} failed,drop it:{}", key, e.what());
          dict.erase(key);
        }
      }
    }

  private:
    synced_tensor_dict &dict;
  };
} // namespace cyy::cxx_lib::pytorch
