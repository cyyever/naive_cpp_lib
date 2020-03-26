#include "synced_tensor_dict.hpp"
#include <chrono>
namespace cyy::cxx_lib::pytorch {

  class synced_tensor_dict::flush_thread final : public cyy::cxx_lib::runnable {
  public:
    explicit flush_thread(synced_tensor_dict &dict_) : dict(dict_) {}
    ~flush_thread() override { stop(); }

  private:
    void run() override {
      while (true) {
        auto save_tasks = dict.pop_expired_data(false, 1);
        if (save_tasks.empty()) {
          std::unique_lock lk(dict.data_mutex);
          dict.flush_cv.wait(lk);
          if (needs_stop()) {
            return;
          }
          continue;
        }
        dict.flush(save_tasks);
      }
    }

  private:
    synced_tensor_dict &dict;
  };

} // namespace cyy::cxx_lib::pytorch
