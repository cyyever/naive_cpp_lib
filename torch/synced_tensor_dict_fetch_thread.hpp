#include <lmdb++.h>
#include <sstream>
#include <stdexcept>

#include <string_view>
#include <torch/csrc/api/include/torch/serialize.h>

#include "log/log.hpp"
#include "synced_tensor_dict.hpp"
namespace cyy::cxx_lib::pytorch {

  class synced_tensor_dict::fetch_thread final : public cyy::cxx_lib::runnable {
  public:
    fetch_thread(synced_tensor_dict &dict_) : dict(dict_) {}
    ~fetch_thread() override { stop(); }

  private:
    void run() override {
      auto env = std::any_cast<std::shared_ptr<lmdb::env>>(dict.storage_handle);
      while (true) {
        auto value_opt =
            dict.fetch_request_queue.pop_front(std::chrono::seconds(1));
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

          auto txn = lmdb::txn::begin(*env, nullptr, MDB_RDONLY);
          auto dbi = lmdb::dbi::open(txn);
          std::string_view lmdb_val;
          auto has_data = dbi.get(txn, key, lmdb_val);
          txn.abort();
          if (!has_data) {
            std::lock_guard lk(dict.data_mutex);
            if (!dict.change_state(key, data_state::LOADING,
                                   data_state::LOAD_FAILED)) {
              continue;
            }
          }
          torch::Tensor value;
          torch::load(value, lmdb_val.data(), lmdb_val.size());
          bool need_flush = false;
          {
            std::lock_guard lk(dict.data_mutex);
            if (!dict.change_state(key, data_state::LOADING,
                                   data_state::IN_MEMORY)) {
              continue;
            }
            dict.data.emplace(key, std::move(value));
            need_flush = dict.need_flush();
          }
          if (need_flush) {
            dict.flush();
          }
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

} // namespace cyy::cxx_lib::pytorch
