/** @file    test_steady_pool.cc
 *  @time    2023/3/4 ~ 下午11:08
 *  @author  Leon
 *
 *  @note    ---
 *
 */

#include <fmt/core.h>
#include <thread>
// #include <threadpool/dynamic_pool.h>
#include <threadpool/steady_pool.h>
#include <utils/printer.h>
#include <utils/tictok.h>
#include <vector>
#include <future>
#include <complex>
#include <Hipe/steady_pond.h>

#ifdef WITH_TBB
#include <tbb/parallel_for.h>
#endif

namespace test {

using namespace std::chrono_literals;
constexpr std::size_t TEST_TASK_NUM = 1000000;

inline float do_math(float a, float b) {
//  std::this_thread::sleep_for(1ms);  // enable this to emulate a long-running task
  return std::cos(std::sin(a)) + std::sin(std::cos(b));
}

void test_submit_task() {
  tp::SteadyThreadPool pool{8};

  // test many tasks
  std::vector<std::future<float>> futures;
  futures.reserve(TEST_TASK_NUM);
  TIC(test_submit_task)
  for (int i = 0; i < futures.capacity(); ++i) {
    futures.emplace_back(pool.submit_task(do_math, 3.14F, 2.71F));
  }
  pool.wait_for_tasks();
  TOK(test_submit_task)

  hipe::SteadyThreadPond pond{8};
  std::vector<std::future<float>> futures2;
  futures.reserve(TEST_TASK_NUM);
  TIC(test_submit_task_Hipe)
  for (int i = 0; i < futures.capacity(); ++i) {
    futures2.emplace_back(pond.submitForReturn([]() { return do_math(3.14F, 2.71F); }));
  }
  pond.waitForTasks();
  TOK(test_submit_task_Hipe)
}

void test_submit_in_batch() {
  tp::SteadyThreadPool pool{8};

  // tasks with return value
  std::vector<std::function<float()>> tasks(TEST_TASK_NUM);
  for (int i = 0; i < tasks.capacity(); ++i) {
    tasks[i] = [] { return do_math(3.14F, 2.71F); };
  }
  TIC(test_submit_in_batch)
  auto futures = pool.submit_in_batch(tasks);
  pool.wait_for_tasks();
  TOK(test_submit_in_batch)
  //  for (auto&& f : futures) { fmt::print("{} ", f.get()); }

}
}  // namespace test


int main() {
  fmt::print("My hardware concurrency -> {}\n", std::thread::hardware_concurrency());
  DividingLine(Start Tests !);
  DividingLine(test_submit_task);
  test::test_submit_task();

  DividingLine(test_submit_in_batch);
  test::test_submit_in_batch();
}