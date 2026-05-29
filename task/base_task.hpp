/*!
 * \file base_task.hpp
 *
 * \brief 任务基类
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <mutex>
#include <optional>

namespace cyy::naive_lib::task {

  //! \brief 綫程任务
  class base_task {
  public:
    base_task() = default;

    virtual ~base_task() = default;

    //! \brief 处理任务
    //! \param timeout 该线程等待超时时间
    bool wait_done(const std::chrono::milliseconds &timeout) {
      // 快速路徑：已經結束的任务無需加鎖
      if (auto s = status.load(std::memory_order_acquire);
          s != task_status::unprocessed) {
        return s == task_status::processed;
      }
      // std::future 不支持并发访问，用互斥量串行化等待者
      std::lock_guard lk(sync_mu);
      if (auto s = status.load(std::memory_order_relaxed);
          s != task_status::unprocessed) {
        return s == task_status::processed;
      }
      if (!_wait_done(timeout)) {
        return false;
      }
      // 只在仍未處理時轉為 processed，避免覆蓋并发的 mark_invalid
      auto expected = task_status::unprocessed;
      status.compare_exchange_strong(expected, task_status::processed,
                                     std::memory_order_acq_rel);
      return status.load(std::memory_order_relaxed) == task_status::processed;
    }

    //! \brief 標記任务作廢，無鎖；已處理的任务不會被改寫
    void mark_invalid() noexcept {
      auto expected = task_status::unprocessed;
      status.compare_exchange_strong(expected, task_status::invalid,
                                     std::memory_order_acq_rel);
    }
    [[nodiscard]] bool is_invalid() const noexcept {
      return status.load(std::memory_order_acquire) == task_status::invalid;
    }
    [[nodiscard]] bool can_process() const noexcept {
      return status.load(std::memory_order_acquire) ==
             task_status::unprocessed;
    }

  protected:
    virtual bool _wait_done(const std::chrono::milliseconds &timeout) = 0;

  private:
    //! \brief 任务状态
    enum class task_status : uint8_t { unprocessed, invalid, processed };
    std::atomic<task_status> status{task_status::unprocessed};
    //! \brief 僅用於串行化 _wait_done 對 std::future 的訪問
    std::mutex sync_mu;
  };

  template <typename ResultType = void> class task : public base_task {
  public:
    task() = default;
    ~task() override = default;

  protected:
    bool _wait_done(const std::chrono::milliseconds &timeout) override {
      if (!future_opt) {
        future_opt = result_promise.get_future();
      }
      if (!future_opt->valid()) {
        return false;
      }
      return future_opt->wait_for(timeout) == std::future_status::ready;
    }

  public:
    std::promise<ResultType> result_promise;

  protected:
    std::optional<std::future<ResultType>> future_opt;
  };

  template <typename ResultType>
  class task_with_result : public task<ResultType> {
    static_assert(!std::is_same_v<ResultType, void>);

  public:
    ~task_with_result() override = default;

    auto const &get_result(const std::chrono::milliseconds &timeout =
                               std::chrono::milliseconds(1)) {
      this->wait_done(timeout);
      return result_opt;
    }

  protected:
    bool _wait_done(const std::chrono::milliseconds &timeout) override {
      if (result_opt.has_value()) {
        return true;
      }
      if (task<ResultType>::_wait_done(timeout)) {
        result_opt = this->future_opt->get();
        this->future_opt.reset();
        return true;
      }

      return false;
    }

  private:
    std::optional<ResultType> result_opt;
  };

  template <typename ArgumentType> class task_with_argument : public task<> {
  public:
    explicit task_with_argument(ArgumentType argument_)
        : argument{std::move(argument_)} {}
    ~task_with_argument() override = default;
    auto const &get_argument() const { return argument; }

  private:
    ArgumentType argument;
  };

  template <typename ArgumentType, typename ResultType>
  class task_with_argument_and_result : public task_with_result<ResultType> {
  public:
    explicit task_with_argument_and_result(ArgumentType argument_)
        : argument{std::move(argument_)} {}
    ~task_with_argument_and_result() override = default;
    auto const &get_argument() const { return argument; }

  private:
    ArgumentType argument;
  };

} // namespace cyy::naive_lib::task
