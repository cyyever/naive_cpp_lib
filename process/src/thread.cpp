/*!
 * \file thread.hpp
 *
 * \brief 封裝thread處理代碼
 * \author cyy
 */

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <memory>
#include <poll.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "log/log.hpp"
#include "thread.hpp"
#include "util/error.hpp"

namespace cyy::cxx_lib::this_thread {
#ifndef _WIN32

  namespace {
    class signal_fd final {
    public:
      signal_fd() {
        sigset_t set_all_sig{};
        sigfillset(&set_all_sig);

        fd = signalfd(-1, &set_all_sig, O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) {
          auto saved_errno = errno;
          LOG_ERROR("signalfd failed:{}", util::errno_to_str(saved_errno));
          throw std::runtime_error("signal_fd failed");
        }
      }
      int get_fd() const { return fd; }

      ~signal_fd() { close(fd); }

    private:
      int fd{-1};
    };
  } // namespace

  std::optional<signalfd_siginfo>
  read_signal(const sigset_t &set,
              const std::optional<std::chrono::milliseconds> &timeout) {
    thread_local static signal_fd fd;

    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::milliseconds>
        end_time;
    if (timeout) {
      end_time = std::chrono::time_point_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now()) +
                 timeout.value();
    }

    bool empty_set = true;
    if (!sigisemptyset(&set)) {
      empty_set = false;
    } else {
      // sigisemptyset不能檢測實時信號，我們自己做檢測
      for (int rt_sig = SIGRTMIN; rt_sig <= SIGRTMAX; rt_sig++) {
        if (sigismember(&set, rt_sig)) {
          empty_set = false;
          break;
        }
      }
    }
    if (empty_set) {
      LOG_ERROR("set is empty");
      return {};
    }

    struct pollfd fds[1]{};

    fds[0].fd = fd.get_fd();
    fds[0].events = POLLIN;

    int res = 0;
    while (true) {
      int timeout_ms = -1;
      if (timeout) {
        timeout_ms = std::max(
            static_cast<int>(
                (end_time -
                 std::chrono::time_point_cast<std::chrono::milliseconds>(
                     std::chrono::steady_clock::now()))
                    .count()),
            0);
      }

      LOG_DEBUG("timeout ms={}", timeout_ms);
      res = poll(fds, 1, timeout_ms);
      if (res < 0 && errno == EINTR) {
        continue;
      }
      break;
    }

    if (res < 0) {
      auto saved_errno = errno;
      LOG_ERROR("ppoll failed:{}", util::errno_to_str(saved_errno));
      return {};
    }
    if (res > 0) {
      if ((fds[0].revents & POLLIN) || (fds[0].revents & POLLHUP)) {
        signalfd_siginfo info{};
        ssize_t res2 = 0;
        while (true) {
          res2 = read(fd.get_fd(), &info, sizeof(info));
          if (res2 < 0 && errno == EINTR) {
            continue;
          }
          break;
        }
        if (res2 > 0) {
          if (res2 != sizeof(info)) {
            LOG_ERROR("read size mismatch {} {}", res2, sizeof(info));
            return {};
          }
          return {info};
        }
        if (res2 < 0) {
          auto saved_errno = errno;
          if (saved_errno != EAGAIN) {
            LOG_ERROR("read failed:{}", util::errno_to_str(saved_errno));
          }
        }
        return {};
      }
    }
    return {};
  }
#endif
} // namespace cyy::cxx_lib::this_thread
