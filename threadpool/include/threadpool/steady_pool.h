/** @file    steady_pool.h
 *  @time    2023/3/4 ~ 下午4:38
 *  @author  Leon
 *
 *  @note    A thread pool with 2 thread-local task queues
 *
 */

#pragma once

#include <thread>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>
#include <future>
#include <threadpool/atomic_spin_lock.h>


namespace tp {  // thread pool

class DoubleQueueThread {
 private:
  // The working thread
  std::thread this_thread{};  // not copyable
  // The 2 working threads
  std::queue<std::function<void()>> tq_work{};
  std::queue<std::function<void()>> tq_buffer{};
  // A spin lock by atomic_flag (lock-free); not copyable or movable.
  tp::atomic_spinlock spin_lock{};
  // mutex
  std::mutex mtx{};
  // total number of tasks remaining
  std::atomic<std::size_t> num_tasks{0};
  // wait for tasks done
  std::condition_variable cv_tasks_done{};  // not movable
  bool waiting{false};

 public:
  DoubleQueueThread() = default;
  explicit DoubleQueueThread(std::thread&& t) : this_thread(std::move(t)) {}
  DoubleQueueThread(DoubleQueueThread&&) = delete;
  DoubleQueueThread(DoubleQueueThread&) = delete;

 public:
  [[nodiscard]] std::size_t get_num_tasks() const { return num_tasks.load(std::memory_order_acquire); }

  void run_tasks() {
    while (!tq_work.empty()) {
      tq_work.front()();  // run the task directly in the working queue
      tq_work.pop();
      num_tasks.fetch_sub(1, std::memory_order_relaxed);  // --num_tasks
    }
  }

  void wait_for_tasks() {
    waiting = true;
    std::unique_lock<std::mutex> lock(mtx);
    cv_tasks_done.wait(lock, [this] { return num_tasks == 0; });
    waiting = false;
  }

  void notify_tasks_done() { cv_tasks_done.notify_one(); }

  [[nodiscard]] bool is_waiting() const { return waiting; }

  void join() { this_thread.join(); }

  void bind_thread(std::thread&& t) { this_thread = std::move(t); }

  bool try_load_tasks() {
    unique_spinlock lck(spin_lock);
    if (tq_buffer.empty()) {  // no more work to do in the buffer queue
      return false;
    } else {
      using std::swap;
      swap(tq_work, tq_buffer);  // ADL
      return true;
    }
  };

  void enqueue(std::function<void()>&& task) {
    unique_spinlock lck(spin_lock);
    tq_buffer.emplace(std::move(task));
    num_tasks.fetch_add(1, std::memory_order_relaxed);  // ++num_tasks
  }

  void enqueue_unsafe(std::function<void()>&& task) {
    tq_buffer.emplace(std::move(task));
    num_tasks.fetch_add(1, std::memory_order_relaxed);  // ++num_tasks
  }

  auto get_lock() { return unique_spinlock(spin_lock); }

  template <typename Container, typename = std::void_t<decltype(std::function<void()>{*std::begin(std::declval<Container>())})>>
  void enqueue(Container&& tasks) {
    unique_spinlock lck(spin_lock);
    std::move(tasks.begin(), tasks.end(), std::back_inserter(tq_buffer));
    num_tasks.fetch_add(tasks.size(), std::memory_order_relaxed);  // num_tasks += tasks.size()
  }

  template <typename Forward_Itr_Begin, typename Forward_Itr_End,
            typename = std::void_t<decltype(std::function<void()>{*std::declval<Forward_Itr_Begin>()})>>
  void enqueue(Forward_Itr_Begin itr_begin, Forward_Itr_End itr_end) {
    unique_spinlock lck(spin_lock);
    std::move(itr_begin, itr_end, std::back_inserter(tq_buffer));
    num_tasks.fetch_add(std::distance(itr_begin, itr_end), std::memory_order_relaxed);  // num_tasks += tasks.size()
  }

};  // class DoubleQueueThread


class SteadyThreadPool {
 private:
  std::vector<DoubleQueueThread> thread_pool;  // or vector<unique_ptr<T>>, as T is not movable
  bool stop{false};                            // to atomic?

 public:
  explicit SteadyThreadPool(std::size_t num_threads = std::thread::hardware_concurrency())
      : stop{false}, thread_pool{num_threads} {
    for (auto& thread : thread_pool) {
      thread.bind_thread(std::thread{&SteadyThreadPool::worker, this, std::ref(thread)});
    }
  }

  ~SteadyThreadPool() {
    wait_for_tasks();
    force_to_stop();
    for (auto& thread : thread_pool) {
      thread.join();
    }
  }

 private:
  void worker(DoubleQueueThread& this_thread) const {
    while (!stop) {
      if (this_thread.try_load_tasks()) {  // buffer queue is not empty
        this_thread.run_tasks();
      } else {  // no more tasks in the buffer queue
        if (this_thread.is_waiting()) {
          this_thread.notify_tasks_done();  // notify the main thread who called wait_for_tasks();
        }
        std::this_thread::yield();  // give up the CPU time slice
      }
    }
  };

  [[nodiscard]] auto& get_least_busy() {
    return *std::min_element(thread_pool.begin(), thread_pool.end(),
                             [](auto& lhs, auto& rhs) { return lhs.get_num_tasks() < rhs.get_num_tasks(); });
  }

 public:
  void wait_for_tasks() {
    for (auto& thread : thread_pool) {
      thread.wait_for_tasks();
    }
  }

  void force_to_stop() { stop = true; }

  template <typename F, typename... Args>
  auto submit_task(F&& func, Args&&... args);

  template <template <typename> typename Container, typename Ret, typename>
  auto submit_in_batch(Container<std::function<Ret()>>& container);

};

// class SteadyThreadPool


template <typename F, typename... Args>
auto SteadyThreadPool::submit_task(F&& func, Args&&... args) {
  using return_type = std::invoke_result_t<F, Args...>;
  auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
  auto future = task->get_future();
  get_least_busy().enqueue([task]() { (*task)(); });
  return future;
}

template <template <typename> typename Container, typename Ret,
          typename = std::void_t<decltype(std::begin(std::declval<Container<std::function<Ret()>>>()))>>
auto SteadyThreadPool::submit_in_batch(Container<std::function<Ret()>>& container) {
  std::shared_ptr<std::packaged_task<Ret()>> sp_task;
  std::vector<std::future<Ret>> futures;
  futures.reserve(container.size());

  for (auto&& function : container) {
    sp_task = std::make_shared<std::packaged_task<Ret()>>(function);
    futures.emplace_back(sp_task->get_future());
    get_least_busy().enqueue([sp_task]() { (*sp_task)(); });
  }

  return futures;
}

}  // namespace tp