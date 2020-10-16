/*!
 * \file process.cpp
 *
 * \brief 封裝進程處理代碼
 * \author cyy
 */
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <set>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "log/log.hpp"
#include "process.hpp"
#include "util/error.hpp"
#include "util/file.hpp"

namespace cyy::cxx_lib::process {

  std::optional<spawn_result> spawn(const spawn_config &config) {
    int pipefd[2]{};

    if (pipe2(pipefd, O_CLOEXEC) != 0) {
      auto saved_errno = errno;
      LOG_ERROR("pipe2 failed:{}", util::errno_to_str(saved_errno));
      return {};
    }
    std::unique_ptr<int, void (*)(int *)> pipefd_RAII(pipefd, [](int *fds) {
      close(fds[0]);
      close(fds[1]);
    });

    int socketfd[2]{};
    std::unique_ptr<int, void (*)(int *)> socketfd_RAII(nullptr, [](int *fds) {
      close(fds[0]);
      close(fds[1]);
    });

    if (config.need_channel) {
      if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, socketfd) != 0) {
        auto saved_errno = errno;
        LOG_ERROR("socketpair failed:{}", util::errno_to_str(saved_errno));
        return {};
      }
      socketfd_RAII.reset(socketfd);
    }

    std::vector<char *> argv;
    for (auto &arg : config.args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    std::vector<char *> envp;
    for (auto &tmp : config.env) {
      envp.push_back(const_cast<char *>(tmp.c_str()));
    }

    std::string channel_fd_str;
    if (config.need_channel) {
      channel_fd_str = "CHANNEL_FD=";
      channel_fd_str += std::to_string(socketfd[1]);
      envp.push_back(const_cast<char *>(channel_fd_str.c_str()));
    }
    envp.push_back(nullptr);

    auto pid = ::fork();
    if (pid == 0) {
      if (config.need_channel) {
        close(socketfd[0]);
      }
      execvpe(config.binary_path.c_str(), argv.data(), envp.data());
      auto saved_errno = errno;

      //通知父進程失敗了
      close(pipefd[0]);
      while (write(pipefd[1], "f", 1) < 0 && errno == EINTR) {
        continue;
      }
      close(pipefd[1]);

      std::set_terminate([]() { _exit(EXIT_FAILURE); });
      LOG_ERROR("exec failed:{}", util::errno_to_str(saved_errno));
      _exit(EXIT_FAILURE);
    } else if (pid < 0) {
      auto saved_errno = errno;
      LOG_ERROR("fork failed:{}", util::errno_to_str(saved_errno));
      return {};
    }

    close(pipefd[1]);

    bool succ = false;
    while (true) {
      char tmp{};
      auto res = read(pipefd[0], &tmp, 1);
      if (res < 0 && errno == EINTR) {
        continue;
      }
      if (res == 0) {
        succ = true;
      }
      break;
    }

    close(pipefd[0]);

    if (succ) {
      if (config.need_channel) {
        close(socketfd[1]);
        [[maybe_unused]] auto tmp=socketfd_RAII.release();
        return {{pid, socketfd[0]}};
      } else {
        return {{pid}};
      }
    }
    return {};
  }

} // namespace cyy::cxx_lib::process
