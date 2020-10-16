/*!
 * \file base_task.hpp
 *
 * \brief 任务基类
 * \author cyy
 * \date 2016-11-09
 */

#pragma once

#include <chrono>
#include <optional>
#include <future>

namespace cyy::cxx_lib::task {

  //! \brief 綫程任务
  class base_task {
  public:
    base_task() = default;

    virtual ~base_task() = default;

    //! \brief 处理任务
    //! \param timeout 该线程等待超时时间
    bool wait_done(const std::chrono::milliseconds &timeout) {
      if (status != task_status::unprocessed) {
        return status == task_status::processed;
      }
      auto res = _wait_done(timeout);
      if (res) {
        status = task_status::processed;
        return true;
      }
      status = task_status::timed_out;
      return false;
    }

    bool has_expired() const { return status == task_status::timed_out; }
    bool has_finished() const { return status == task_status::processed; }

  private:
    virtual bool _wait_done(const std::chrono::milliseconds &timeout) = 0;

  private:
    //! \brief 任务状态
    enum class task_status : uint8_t { unprocessed, timed_out, processed };

  private:
    std::atomic<task_status> status{task_status::unprocessed};
  };

  template <typename ResultType = void>
  class task_with_result : public base_task {
  public:
    ~task_with_result() override = default;

    auto const &get_result(const std::chrono::milliseconds &timeout =
                               std::chrono::milliseconds(1)) {
      _wait_done(timeout);
      return result_opt;
    }

  private:
    bool _wait_done(const std::chrono::milliseconds &timeout) override {
      if (result_opt.has_value()) {
        return true;
      }
      auto future = result_promise.get_future();
      if (!future.valid()) {
        return false;
      }
      if (has_expired()) {
        return false;
      }
      if (future.wait_for(timeout) == std::future_status::ready) {
        result_opt = future.get();
        return true;
      }
      return false;
    }

  public:
    std::promise<ResultType> result_promise;

  private:
    std::optional<ResultType> result_opt;
  };

  template <typename ArgumentType, typename ResultType = void>
  class task_with_argument_and_result : public task_with_result<ResultType> {
  public:
    explicit task_with_argument_and_result(ArgumentType argument_)
        : argument{std::move(argument_)} {}
    ~task_with_argument_and_result() override = default;
    auto const &get_argument() const { return argument; }

  private:
    ArgumentType argument;
  };

} // namespace cyy::cxx_lib::task
