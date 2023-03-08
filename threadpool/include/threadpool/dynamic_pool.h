/** @file    dynamic_pool.h
 *  @time    2023/2/26 ~ 下午10:16
 *  @author  Leon
 *
 *  @note    A normal thread pool with an unbounded queue
 *
 */

#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>


namespace tp {  // thread pool

class DynamicThreadPool {
 private:  // Variables
  // Flag to stop the thread pool forever
  bool stop{false};  // TODO: use atomic?
  // The working threads
  std::vector<std::thread> thread_pool{};
  // The shared queue contains tasks
  std::queue<std::function<void()>> task_queue{};
  // mutex
  std::mutex mtx{};
  // conditional variable for awake workers
  std::condition_variable cv_awake{};
  // conditional variable for wait_for_tasks()
  std::condition_variable cv_tasks_done{};
  // to indicate the main thread is waiting for tasks done
  bool waiting{false};
  // total number of tasks remaining
  std::atomic<std::size_t> num_tasks{0};

 public:  // constructor and destructor
  explicit DynamicThreadPool(std::size_t num_threads = std::thread::hardware_concurrency()) : stop{false} {
    thread_pool.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
      thread_pool.emplace_back(&DynamicThreadPool::worker, this);
    }
  }

  ~DynamicThreadPool() {
    wait_for_tasks();
    force_to_stop();
    for (auto&& t : thread_pool) {
      t.join();
    }
  }

 public:  // public functions
  template <typename F, typename... Args>
  auto submit_task(F&& func, Args&&... args);

  template <template <typename> typename Container, typename Ret, typename>
  auto submit_in_batch(Container<std::function<Ret()>>& container);

  template <typename Container, typename>
  void submit_in_batch(Container&& container);

  [[nodiscard]] std::size_t get_num_threads() const { return thread_pool.size(); }

  void force_to_stop() {
    stop = true;  // abandon remaining tasks!
    cv_awake.notify_all();
  }

  void wait_for_tasks() {
    waiting = true;
    std::unique_lock<std::mutex> lck{mtx};
    cv_tasks_done.wait(lck, [this]() { return num_tasks == 0; });
    waiting = false;
  }

 private:
  void worker();

};


void DynamicThreadPool::worker() {
  std::function<void()> task;

  while (true) {
    std::unique_lock<std::mutex> lck{mtx};
    cv_awake.wait(lck, [this]() { return !task_queue.empty() || stop; });

    [[likely]] if (!stop) {
      task = std::move(task_queue.front());
      task_queue.pop();
      lck.unlock();
      task();
      num_tasks.fetch_sub(1, std::memory_order_relaxed);  // --num_tasks
      if (waiting) {
        cv_tasks_done.notify_one();
      }

    } else {
      break;
    }
  }
}

template <typename F, typename... Args>
auto DynamicThreadPool::submit_task(F&& func, Args&&... args) {
  using return_type = std::invoke_result_t<F, Args...>;
  // packaged_task is a callable object of void() type, but not copyable; so we need to wrap it in a shared_ptr,
  // which is copied by the lambda function and then moved into the queue.
  auto sp_task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(func), std::forward<Args>(args)...));
  std::future<return_type> future{sp_task->get_future()};

  {
    std::unique_lock<std::mutex> lck{mtx};
    task_queue.emplace([sp_task]() { (*sp_task)(); });  //  lambda: void() type and copyable
  }
  num_tasks.fetch_add(1, std::memory_order_relaxed); // ++num_tasks
  cv_awake.notify_one();
  return future;
}

template <template <typename> typename Container, typename Ret,
          typename = std::void_t<decltype(std::begin(std::declval<Container<std::function<Ret()>>>()))>>
auto DynamicThreadPool::submit_in_batch(Container<std::function<Ret()>>& container) {
  std::shared_ptr<std::packaged_task<Ret()>> sp_task;
  std::vector<std::future<Ret>> futures;
  futures.reserve(container.size());

  std::unique_lock<std::mutex> lck{mtx};
  for (auto&& function : container) {
    sp_task = std::make_shared<std::packaged_task<Ret()>>(std::move(function));
    futures.emplace_back(sp_task->get_future());
    task_queue.emplace([sp_task]() { (*sp_task)(); });
  }
  lck.unlock();
  num_tasks.fetch_add(container.size(), std::memory_order_relaxed); // += container.size();
  cv_awake.notify_all();

  return futures;
}

template <typename Container,
          typename = std::void_t<decltype(std::function<void()>{*std::begin(std::declval<Container>())})>>
void DynamicThreadPool::submit_in_batch(Container&& container) {
  std::unique_lock<std::mutex> lck{mtx};
  std::move(std::begin(container), std::end(container), std::back_inserter(task_queue));
  lck.unlock();
  num_tasks.fetch_add(container.size(), std::memory_order_relaxed); // += container.size();
  cv_awake.notify_all();
}

}  // namespace tp
