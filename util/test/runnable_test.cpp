/*!
 * \file runnable_test.cpp
 *
 * \brief 测试runnable相关函数
 * \author cyy
 */

#include <doctest/doctest.h>

#include "log/log.hpp"
#include "util/runnable.hpp"

class test_class : public cyy::naive_lib::runnable {
public:
  test_class() = default;

  ~test_class() override { stop(); }

  void test_wait_stop() {
    start();
    CHECK(!wait_stop(std::chrono::seconds(1)));
    stop();
    wait_stop();
  }
  void test_stop() {
    stop();
    stop();
  }
  void test_restart() {
    for (size_t i = 0; i < 2; i++) {
      start();
      stop();
    }
  }

#if defined(__linux__) || defined(__FreeBSD__)
  void test_name() {
    thread_name = "test_thread";
    start(thread_name);
    stop();
    thread_name.clear();
  }
#endif

  void test_exception() {
    throw_exception = true;
    bool thrown = false;
    std::string str_err;
    exception_callback = [&thrown, &str_err](const std::exception &e) {
      LOG_ERROR("exception callback:{}", e.what());
      str_err = e.what();
      thrown = true;
    };
    start();
    stop();
    REQUIRE(thrown);
    CHECK_EQ("test exception", str_err);
    throw_exception = false;
  }

private:
  void run(const std::stop_token & /*st*/) override {
    LOG_ERROR("run thd");
#if defined(__linux__) || defined(__FreeBSD__)
    if (!thread_name.empty()) {
      char buf[1024]{};
      auto err = pthread_getname_np(pthread_self(), buf, sizeof(buf));
      REQUIRE_EQ(err, 0);
      CHECK_EQ(thread_name, buf);
    }
#endif
    do {
      if (throw_exception) {
        throw std::runtime_error("test exception");
      }
    } while (!needs_stop());
  }

  std::string thread_name;
  bool throw_exception{false};
};

TEST_CASE("runnable") {
  test_class tester;
  SUBCASE("stop") { tester.test_stop(); }

  SUBCASE("restart") { tester.test_restart(); }
  SUBCASE("start twice") {
    tester.start();
    CHECK_THROWS(tester.start());
  }

#if defined(__linux__) || defined(__FreeBSD__)
  SUBCASE("name") { tester.test_name(); }
#endif

  SUBCASE("exception") { tester.test_exception(); }
}
