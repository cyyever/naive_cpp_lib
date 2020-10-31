/*!
 * \file monitor.cpp
 *
 * \brief 進程監控器
 * \author cyy
 */
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <vector>

#include <sys/signalfd.h>
#include <sys/wait.h>

#include "log/log.hpp"
#include "process.hpp"
#include "thread.hpp"
#include "util/error.hpp"
#include "util/file.hpp"
#include "util/runnable.hpp"

namespace cyy::naive_lib::process::monitor {

  namespace {
    class monitor_thread final : public cyy::naive_lib::runnable {
    public:
      using exec_argument_tuple_type =
          std::tuple<std::string, std::vector<std::string>,
                     std::vector<std::string>>;
      monitor_thread() {
        struct sigaction sa {};

        //先獲取舊的信號處理
        if (sigaction(SIGCHLD, nullptr, &sa) != 0) {
          auto saved_errno = errno;
          LOG_ERROR("sigaction SIGCHLD failed:{}",
                    util::errno_to_str(saved_errno));
          throw std::runtime_error("sigaction SIGCHLD failed");
        }

        //我們不能覆蓋信號處理函數，不安全
        if (sa.sa_handler != SIG_DFL) {
          throw std::runtime_error("signal hanlder for SIGCHLD has previously "
                                   "been set,we can't overwrite it");
        }

        start();
      }
      ~monitor_thread() override {
        stop([this] {
          sigval val{};
          // c++的zero initialization对union只能初始化first
          // member，所以我们要明确地用memset
          memset(&val, 0, sizeof(val));
          auto res = pthread_sigqueue(monitor_pthread_t, SIGCHLD, val);
          if (res != 0) {
            LOG_ERROR("pthread_sigqueue failed:{}", util::errno_to_str(res));
          }
        });
      }

      prog_id register_child(const spawn_config &config,
                             const spawn_result &result,
                             const std::chrono::milliseconds &retry_duration,
                             size_t max_retry_count) {
        std::lock_guard<std::mutex> lock(children_mutex);
        auto child_id = next_prog_id;
        next_prog_id++;

        monitored_children[result.pid] = {
            child_id,        true,
            config,          result,
            retry_duration,  0,
            max_retry_count, std::chrono::steady_clock::now()};
        return child_id;
      }

      bool signal_child(prog_id id, int signo) const {
        std::optional<pid_t> pid;
        std::lock_guard<std::mutex> lock(children_mutex);
        for (const auto &p : monitored_children) {
          if (p.second.id == id) {
            pid = p.first;
            break;
          }
        }

        if (!pid) {
          LOG_ERROR("no child for program id {}", id);
          return false;
        }

        sigval val{};
        // c++的zero initialization对union只能初始化first
        // member，所以我们要明确地用memset
        memset(&val, 0, sizeof(val));
        if (sigqueue(pid.value(), signo, val) != 0) {
          auto saved_errno = errno;
          LOG_ERROR("sigqueue failed:{}", util::errno_to_str(saved_errno));
          return false;
        }
        return true;
      }

      std::optional<size_t> write_child(prog_id id,
                                        gsl::not_null<const void *> data,
                                        size_t data_len) const {
        std::lock_guard<std::mutex> lock(children_mutex);
        for (const auto &p : monitored_children) {
          if (p.second.id == id) {
            if (p.second.result.channel_fd) {
              return io::write(p.second.result.channel_fd.value(), data,
                               data_len);
            } else {
              LOG_ERROR("no channel for program id {}", id);
              return {};
            }
          }
        }

        LOG_ERROR("no child for program id {}", id);
        return {};
      }

      std::optional<std::vector<std::byte>>
      read_child(prog_id id, size_t max_read_size) const {
        std::lock_guard<std::mutex> lock(children_mutex);
        for (const auto &p : monitored_children) {
          if (p.second.id == id) {
            if (p.second.result.channel_fd) {
              auto res =
                  io::read(p.second.result.channel_fd.value(), max_read_size);
              if (res.first) {
                return res.second;
              }
              return {};
            } else {
              LOG_ERROR("no channel for program id {}", id);
              return {};
            }
          }
        }

        LOG_ERROR("no child for program id {}", id);
        return {};
      }

      bool child_exist(prog_id id) const {
        pid_t child_pid = -1;
        std::lock_guard<std::mutex> lock(children_mutex);
        for (const auto &[pid, setting] : monitored_children) {
          if (setting.id == id) {
            child_pid = pid;
            break;
          }
        }

        if (child_pid == -1) {
          return false;
        }
        auto res = kill(child_pid, 0);
        if (res != 0) {
          auto saved_errno = errno;
          LOG_DEBUG("kill failed:{}", util::errno_to_str(saved_errno));
          return false;
        }
        return true;
      }

    private:
      struct child_setting {
        prog_id id{};
        bool check_exist{true};
        spawn_config config{};
        spawn_result result{};
        std::chrono::milliseconds retry_duration{};
        size_t retry_count{};
        size_t max_retry_count{};
        std::chrono::time_point<std::chrono::steady_clock> last_spawn_time{};
      };

    private:
      void waitpid_wrapper(pid_t pid) {
        while (true) {
          int wstatus = 0;
          auto res_pid = waitpid(pid, &wstatus, WNOHANG);
          if (res_pid > 0) {
            LOG_DEBUG("zombie pid is {}", res_pid);
            std::lock_guard<std::mutex> lock(children_mutex);
            auto it = monitored_children.find(res_pid);
            if (it != monitored_children.end()) {
              deal_zombie(wstatus, it->second);
              monitored_children.erase(it);
            }
            if (pid == -1) {
              //繼續收割靈魂
              continue;
            }
            return;
          }

          if (res_pid == 0) { //孩子還在
            if (pid != -1) {
              std::lock_guard<std::mutex> lock(children_mutex);
              auto it = monitored_children.find(pid);
              if (it != monitored_children.end()) {
                it->second.check_exist = false;
              }
            }
            return;
          }

          if (errno == EINTR) {
            continue;
          }

          int saved_errno = errno;

          if (pid != -1) {
            std::lock_guard<std::mutex> lock(children_mutex);
            auto it = monitored_children.find(pid);
            if (it != monitored_children.end()) {
              if (saved_errno == ECHILD) {
                //孩子已經掛了
                deal_zombie(wstatus, it->second);
              } else {
                LOG_ERROR("waitpid [{}] failed,ignore this child:{}", it->first,
                          util::errno_to_str(saved_errno));
              }
              monitored_children.erase(it);
            }
            return;
          }

          if (saved_errno != ECHILD) {
            LOG_ERROR("waitpid failed:{}", util::errno_to_str(saved_errno));
          }
          break;
        }
      }

      auto steady_now() const {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now());
      }

      //查詢最近的要啓動的進程,獲取等待時間
      std::optional<std::chrono::milliseconds> get_wait_timeout() const {
        if (!retried_children.empty()) {
          auto now = steady_now();
          if (retried_children.begin()->first >= now) {
            return {retried_children.begin()->first - now};
          } else {
            return {std::chrono::milliseconds(0)};
          }
        }
        return {};
      }

      void run() override {
        monitor_pthread_t = pthread_self();
        sigset_t set_all_sig{};
        sigfillset(&set_all_sig);

        auto res = pthread_sigmask(SIG_SETMASK, &set_all_sig, nullptr);

        if (res != 0) {
          LOG_ERROR("pthread_sigmask failed:{}", util::errno_to_str(res));
          return;
        }

        sigset_t set_only_chld{};
        sigemptyset(&set_only_chld);
        sigaddset(&set_only_chld, SIGCHLD);

        while (true) {
          auto siginfo_opt =
              this_thread::read_signal(set_only_chld, get_wait_timeout());

          if (needs_stop()) {
            break;
          }

          if (siginfo_opt) {
            LOG_DEBUG("ssi_pid is {},ssi_code is {}", siginfo_opt->ssi_pid,
                      siginfo_opt->ssi_code);
            std::vector<pid_t> pids_to_wait;
            {
              std::lock_guard<std::mutex> lock(children_mutex);
              for (auto const &p : monitored_children) {
                if (p.second.check_exist) {
                  pids_to_wait.push_back(p.first);
                }
              }
            }

            for (auto const &pid : pids_to_wait) {
              LOG_DEBUG("check new child {}", pid);

              waitpid_wrapper(pid);
            }
            //接下來处理任意子進程結束
            waitpid_wrapper(-1);
          }

          while (true) {
            auto wait_timeout = get_wait_timeout();

            if (!wait_timeout || wait_timeout->count() != 0) {
              break;
            }

            //最近的要啓動的進程
            auto it = retried_children.begin();
            auto setting = it->second;
            retried_children.erase(it);

            auto now = steady_now();

            setting.retry_count++;
            LOG_INFO("start child [{}], retry count {}",
                     setting.config.binary_path, setting.retry_count);
            if (setting.retry_count > setting.max_retry_count) {
              LOG_ERROR("start child [{}] failed,reach max retry count {},so "
                        "we give up",
                        setting.config.binary_path, setting.max_retry_count);
              continue;
            }

            LOG_DEBUG("child [{}] retry count is {}",
                      setting.config.binary_path, setting.retry_count);

            auto result_opt = spawn(setting.config);
            setting.last_spawn_time = now;
            if (!result_opt) {
              LOG_ERROR("start child [{}] failed,retry count is {}",
                        setting.config.binary_path, setting.retry_count);
              retried_children.insert({now + setting.retry_duration, setting});
            } else {
              LOG_INFO("restart child [{}], pid {}", setting.config.binary_path,
                       result_opt->pid);
              setting.check_exist = true;
              setting.result = *result_opt;
              std::lock_guard<std::mutex> lock(children_mutex);
              monitored_children.emplace(result_opt->pid, setting);
            }
          }
        }
      }

      void deal_zombie(int wstatus, child_setting setting) {
        LOG_INFO("deal zombie {}", setting.config.binary_path);
        if (setting.result.channel_fd) {
          close(setting.result.channel_fd.value());
          setting.result.channel_fd.reset();
        }
        if (WIFEXITED(wstatus)) {
          //如果正常結束，我們不重啓
          if (WEXITSTATUS(wstatus) == EXIT_SUCCESS) {
            return;
          }
          LOG_ERROR("child [{}] exit with code {}", setting.config.binary_path,
                    WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
          LOG_ERROR("child [{}] killed by signal {}",
                    setting.config.binary_path, WTERMSIG(wstatus));
          if (WTERMSIG(wstatus) == SIGKILL) {
            LOG_ERROR("we will not restart child [{}] because it is killed by "
                      "SIGKILL",
                      setting.config.binary_path);
            return;
          }
        } else { // stop 或 resume 我們不關心
          return;
        }

        auto now = steady_now();
        if (std::chrono::duration_cast<std::chrono::hours>(
                now - setting.last_spawn_time)
                .count() > 1) {
          LOG_DEBUG(
              "child [{}] exists for more than one hour,reset retry count",
              setting.config.binary_path);
          setting.retry_count = 0;
        }

        retried_children.emplace(now + setting.retry_duration, setting);
      }

    private:
      mutable std::mutex children_mutex;
      std::map<pid_t, child_setting> monitored_children;
      std::multimap<std::chrono::time_point<std::chrono::steady_clock,
                                            std::chrono::milliseconds>,
                    child_setting>
          retried_children;
      prog_id next_prog_id{};
      pthread_t monitor_pthread_t{};
    };

    monitor_thread &get_monitor_thread() {
      static monitor_thread thd;
      return thd;
    }

  } // namespace

  bool init() {
    //我們需要在程序啓動時阻塞SIGCHLD,確保只有monitor_thread能調用到
    sigset_t set_only_chld{};
    sigemptyset(&set_only_chld);
    sigaddset(&set_only_chld, SIGCHLD);

    auto res = pthread_sigmask(SIG_BLOCK, &set_only_chld, nullptr);
    if (res != 0) {
      LOG_ERROR("pthread_sigmask failed:{}", util::errno_to_str(res));
      return false;
    }
    return true;
  }

  std::optional<prog_id> start_process(const spawn_config &config,
                                       std::chrono::milliseconds retry_duration,
                                       size_t max_retry_count) {
    auto res_opt = spawn(config);
    if (!res_opt) {
      return {};
    }
    return get_monitor_thread().register_child(config, res_opt.value(),
                                               retry_duration, max_retry_count);
  }

  bool signal_monitored_process(prog_id id, int signo) {
    return get_monitor_thread().signal_child(id, signo);
  }

  std::optional<size_t>
  write_monitored_process(prog_id id, gsl::not_null<const void *> data,
                          size_t data_len) {
    return get_monitor_thread().write_child(id, data, data_len);
  }

  std::optional<std::vector<std::byte>>
  read_monitored_process(prog_id id, size_t max_read_size) {
    return get_monitor_thread().read_child(id, max_read_size);
  }

  bool monitored_process_exist(prog_id id) {
    return get_monitor_thread().child_exist(id);
  }

} // namespace cyy::naive_lib::process::monitor
