#include <fmt/core.h>
#include <thread>
#include <threadpool/dynamic_pool.h>
#include <utils/printer.h>
#include <utils/tictok.h>
#include <vector>
#include <future>
#include <complex>

#ifdef WITH_TBB
#include <tbb/parallel_for.h>
#endif

namespace test {

using namespace std::chrono_literals;
constexpr std::size_t TEST_TASK_NUM = 100000;

float do_math(float a, float b) {
  //  std::this_thread::sleep_for(1ms);  // enable this to emulate a long-running task
  return std::cos(std::sin(a)) + std::sin(std::cos(b));
}

void test_submit_task() {
  tp::DynamicThreadPool pool{};

  // test many tasks
  std::vector<std::future<float>> futures;
  futures.reserve(TEST_TASK_NUM);
  TIC(test_submit_task)
  for (int i = 0; i < futures.capacity(); ++i) {
    futures.emplace_back(pool.submit_task(do_math, 3.14F, 2.71F));
  }
  pool.wait_for_tasks();
  TOK(test_submit_task)
  //  for (auto&& f : futures) { fmt::print("{} ", f.get()); }
#ifdef WITH_TBB
  TIC(test_TBB)
  std::vector<float> ans(TEST_TASK_NUM);
  tbb::parallel_for(tbb::blocked_range<std::size_t>(0, futures.capacity()), [&](const tbb::blocked_range<std::size_t>& r) {
    for (std::size_t i = r.begin(); i != r.end(); ++i) {
      ans[i] = do_math(3.14F, 2.71F);
    }
  });
  TOK(test_TBB)
#endif

  //   test void return type and lambda
  auto future1 = pool.submit_task([]() { fmt::print("Hello, World!\n"); });
  future1.get();
}


void test_submit_in_batch() {
  tp::DynamicThreadPool pool{};

  // tasks with return value
  std::vector<std::function<float()>> tasks(TEST_TASK_NUM);
  TIC(test_submit_in_batch)
  for (auto&& task : tasks) {
    task = [] { return do_math(3.14F, 2.71F); };
  }
  auto futures = pool.submit_in_batch(tasks);
  pool.wait_for_tasks();
  TOK(test_submit_in_batch);

  // tasks with void return type and convert to std::function<void()>
  std::vector<std::function<void()>> tasks2(TEST_TASK_NUM, []() { do_math(3.14F, 2.71F); });
  TIC(test_submit_in_batch_type_erasure)
  pool.submit_in_batch(tasks2);
  pool.wait_for_tasks();
  TOK(test_submit_in_batch_type_erasure);
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

/*
 * 现象:
 * 1. 每个任务十分快速时, 线程池的线程数越少性能越好, 因为多线程争抢锁的开销比较小, 此时性能总是低于 TBB
 * 2. 每个任务耗时越长, 线程池的线程数越多性能越好, 因为多线程争抢失败的概率大大降低, 高线程数时性能高于 TBB
 *
 * 3. 批量提交有利于任务很快的场景, 线程数很少时, 两种提交相差不大
 */