/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_THREAD_SAFE_QUEUE_HPP__
#define __HIPPO_THREAD_SAFE_QUEUE_HPP__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "hippo_namespace.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;
    ThreadSafeQueue(const ThreadSafeQueue& other) = delete;

    ~ThreadSafeQueue() { BreakAllWait(); }

    void Enqueue(const T& element) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace(element);
        cv_.notify_one();
    }

    bool Dequeue(T* element) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        *element = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    bool WaitDequeue(T* element) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return break_all_wait_ || !queue_.empty(); });
        if (break_all_wait_) {
            return false;
        }
        *element = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    typename std::queue<T>::size_type Size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void BreakAllWait() {
        break_all_wait_ = true;
        cv_.notify_all();
    }

private:
    volatile bool break_all_wait_ = false;
    std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cv_;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_THREAD_SAFE_QUEUE_HPP__
