/*!
 * \file base_task.hpp
 *
 * \brief 任务基类
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include <chrono>
#include <future>
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
      std::lock_guard lk(sync_mu);
      if (status != task_status::unprocessed) {
        return status == task_status::processed;
      }
      auto res = _wait_done(timeout);
      if (res) {
        status = task_status::processed;
      }
      return res;
    }

    void mark_invalid() {
      std::lock_guard lk(sync_mu);
      status = task_status::invalid;
    }
    bool is_invalid() const { return status == task_status::invalid; }
    bool can_process() const { return status == task_status::unprocessed; }

  protected:
    virtual bool _wait_done(const std::chrono::milliseconds &timeout) = 0;

  private:
    //! \brief 任务状态
    enum class task_status : uint8_t { unprocessed, invalid, processed };
    std::atomic<task_status> status{task_status::unprocessed};
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
