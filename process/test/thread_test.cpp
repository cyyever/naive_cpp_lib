/*!
 * \file thread_test.cpp
 *
 * \author cyy
 */

#include <vector>

#include <doctest/doctest.h>

#include "process/src/thread.hpp"

TEST_CASE("no_signal") {
  CHECK(!::cyy::cxx_lib::this_thread::read_signal({}, {}));
}

TEST_CASE("one_signal") {

  sigset_t all_set{};
  sigfillset(&all_set);
  auto res = pthread_sigmask(SIG_BLOCK, &all_set, nullptr);
  auto errmsg = strerror(res);
  CHECK_MESSAGE(res == 0, errmsg);

  int signo = SIGRTMIN;
  sigval val{};
  // c++的zero initialization对union只能初始化first
  // member，所以我们要明确地用memset
  memset(&val, 0, sizeof(val));
  res = pthread_sigqueue(pthread_self(), signo, val);
  errmsg = strerror(res);
  CHECK_MESSAGE(res == 0, errmsg);

  sigemptyset(&all_set);
  res = sigaddset(&all_set, signo);
  auto saved_errno = errno;
  errmsg = strerror(saved_errno);
  CHECK_MESSAGE(res == 0, errmsg);
  auto siginfo = ::cyy::cxx_lib::this_thread::read_signal(all_set, {});

  CHECK(siginfo);
  CHECK(siginfo->ssi_signo == signo);
}
