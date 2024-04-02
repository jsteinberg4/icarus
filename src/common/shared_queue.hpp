#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

namespace common {

/**
 * @brief Thread safe wrapper around std::queue
 *
 * Aims to match the std::queue API as closely as possible, except where that
 * would break thread safety.
 *
 *
 * Comparison with std::queue API
 * Member functions:
 *  TODO: operator= assigns values to the container adaptor (public)
 *  ----
 *  Element access:
 *  ---> These may not be easy to make thread safe!!
 *  ---> Verdict: Maybe possible, very hard. Omitting
 *  (-) front | access the first element | (public member function)
 *  (-) back | access the last element | (public member function)
 *  ---
 * TODO:  Capacity:
 *  (✓) empty | checks whether the container adaptor is empty | (public)
 *  (✓) size | returns the number of elements | (public member)
 *  ----
 *  TODO:
 *  Modifiers:
 *  ( ) push | inserts element at the end | (public member function)
 *  (✓) emplace (C++11) | constructs element in-place at the end | (public)
 *  (+) pop | removes the first element | (public member function)
 *  (-) swap (C++11) | swaps the contents | (public member function)
 *
 * @tparam T type param for internal queue. ie. std::queue<T>
 */
template <typename T> class shared_queue {
private:
  std::queue<T> queue;
  std::mutex mtx;
  std::condition_variable has_items;

public:
  /**
   * @brief Create empty queue<T> and default thread safety primitives
   *
   * Marks host thread ID as 0
   */
  shared_queue<T>() noexcept : shared_queue<T>(0){};

  /**
   * @brief Create empty queue<T> and default thread safety primitives
   */
  shared_queue<T>(uint32_t host_tid) noexcept : queue(), mtx(), has_items(){};

  /**
   * @brief Provide access to the lock protecting this queue
   *
   * @return reference to lock
   */
  inline std::mutex &getLock() noexcept { return this->mtx; }

  /**
   * @brief Provides access to this queue's guard
   *
   * @return reference to the queue's guard
   */
  inline std::condition_variable &getGuard() noexcept {
    return this->has_items;
  }

  /**
   * @brief Current queue size
   * Note: Read only; does not lock
   *
   * @return count of items on the queue. May be inconsistent
   */
  inline uint32_t size() const { return this->queue.size(); }

  /**
   * Note: Read only; does not lock
   *
   * @return true if no items in queue
   */
  inline bool empty() const { return this->queue.empty(); };

  /**
   * @brief Add element to the front by reference
   *
   *
   * @param value element reference
   */
  void push(const T &value) {
    std::unique_lock<std::mutex> lock{this->mtx};
    this->log.Trace("push(const T&): Pushing new value");
    this->queue.push(value);
    this->has_items.notify_all();
    // (implicit) release lock
  }

  /**
   * @brief Move a value into the queue
   *
   * equivalent to std::move(T &&value)
   *
   * @param value element to move
   */
  void push(T &&value) {
    std::unique_lock<std::mutex> lock{this->mtx};
    this->queue.push(value);
    this->has_items.notify_all();
  }

  /**
   * @brief Remove and return front of the queue. Blocks until values are
   * present.
   *
   * Combines std::queue::front and std::queue::pop. Another thread could
   * std::queue::pop between calls to front() and pop(), so combining was
   * necessary for thread safety.
   *
   * @return pair of (element reference, bool). Element reference is valid iff
   * std::pair::second is true. This prevents undefined behavior when the queue
   * is empty.
   */
  std::unique_ptr<T> pop() {
    std::unique_lock<std::mutex> lock{this->mtx};

    while (this->empty()) { // Block until items arrive
      this->has_items.wait(lock);
    } // Wait will re-acquire the lock

    // Remove and return the head
    std::unique_ptr<T> popped =
        std::unique_ptr<T>(new T(std::move(this->queue.front())));
    this->queue.pop();
    return popped;
  }

  /**
   * @brief Push a new element to the end of the queue. Constructed inplace.
   *
   * Wraps std::queue::emplace
   *
   * @tparam types of T ctor arguments
   * @param args arguments to forward to elment constructor
   */
  template <class... Args> void emplace(Args &&...args) {
    std::unique_lock<std::mutex> lock{this->mtx};
    this->queue.emplace(std::forward<Args>(args)...);
    this->has_items.notify_all();
    // (implicit) release lock!
  }
};
} // namespace common
