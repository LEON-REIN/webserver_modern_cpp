/** @file    atomic_spin_lock.h
 *  @time    2023/3/4 ~ 下午10:03
 *  @author  Leon
 *
 *  @note    A spin lock implementation using atomic_flag
 *
 */

#pragma once

#include <atomic>

namespace tp {
class atomic_spinlock {
 private:
  std::atomic_flag atomic_flag_ = ATOMIC_FLAG_INIT;

 public:
  void lock() {
    while (atomic_flag_.test_and_set(std::memory_order_acquire)) {
      // spin, waiting for the lock to be released (set to false) by other threads
    }
  }

  void unlock() { atomic_flag_.clear(std::memory_order_release); }

  bool try_lock() { return !atomic_flag_.test_and_set(std::memory_order_acquire); }
};


class unique_spinlock {
 private:
  atomic_spinlock& lock_;
  bool owns_lock_{false};

 public:
  explicit unique_spinlock(atomic_spinlock& lock) : lock_(lock), owns_lock_(true) { lock_.lock(); }

  void unlock() {
    if (owns_lock_) {
      lock_.unlock();
      owns_lock_ = false;
    }
  }

  ~unique_spinlock() {
    if (owns_lock_) {
      owns_lock_ = false;
      lock_.unlock();
    }
  }
};


}  // namespace tp
