/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_BOUNDED_QUEUE_HPP__
#define __HIPPO_BOUNDED_QUEUE_HPP__

#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>

#include "hippo_namespace.hpp"
#include "hippo_wati_strategy.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

template <typename T>
class BoundedQueue {
public:
    using value_type = T;
    using size_type = uint64_t;

public:
    BoundedQueue() {}
    BoundedQueue& operator=(const BoundedQueue& other) = delete;
    BoundedQueue(const BoundedQueue& other) = delete;
    ~BoundedQueue() {
        if (wait_strategy_) {
            BreakAllWait();
        }
        if (pool_) {
            for (uint64_t i = 0; i < pool_size_; ++i) {
                pool_[i].~T();
            }
            std::free(pool_);
        }
    }
    bool Init(uint64_t size) { return Init(size, new SleepWaitStrategy()); }
    bool Init(uint64_t size, WaitStrategy* strategy) {
        // Head and tail each occupy a space
        pool_size_ = size + 2;
        pool_ = reinterpret_cast<T*>(std::calloc(pool_size_, sizeof(T)));
        if (pool_ == nullptr) {
            return false;
        }
        for (uint64_t i = 0; i < pool_size_; ++i) {
            new (&(pool_[i])) T();
        }
        wait_strategy_.reset(strategy);
        return true;
    }
    bool Enqueue(const T& element) {
        uint64_t new_tail = 0;
        uint64_t old_commit = 0;
        uint64_t old_tail = tail_.load(std::memory_order_acquire);
        do {
            new_tail = old_tail + 1;
            if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
                return false;
            }
        } while (
            !tail_.compare_exchange_weak(old_tail, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed));
        pool_[GetIndex(old_tail)] = element;
        do {
            old_commit = old_tail;
        } while (hippo_unlikely(!commit_.compare_exchange_weak(old_commit, new_tail, std::memory_order_acq_rel,
                                                               std::memory_order_relaxed)));
        wait_strategy_->NotifyOne();
        return true;
    }
    bool Enqueue(T&& element) {
        uint64_t new_tail = 0;
        uint64_t old_commit = 0;
        uint64_t old_tail = tail_.load(std::memory_order_acquire);
        do {
            new_tail = old_tail + 1;
            if (GetIndex(new_tail) == GetIndex(head_.load(std::memory_order_acquire))) {
                return false;
            }
        } while (
            !tail_.compare_exchange_weak(old_tail, new_tail, std::memory_order_acq_rel, std::memory_order_relaxed));
        pool_[GetIndex(old_tail)] = std::move(element);
        do {
            old_commit = old_tail;
        } while (hippo_unlikely(!commit_.compare_exchange_weak(old_commit, new_tail, std::memory_order_acq_rel,
                                                               std::memory_order_relaxed)));
        wait_strategy_->NotifyOne();
        return true;
    }
    bool WaitEnqueue(const T& element) {
        while (!break_all_wait_) {
            if (Enqueue(element)) {
                return true;
            }
            if (wait_strategy_->EmptyWait()) {
                continue;
            }
            // wait timeout
            break;
        }

        return false;
    }
    bool WaitEnqueue(T&& element) {
        while (!break_all_wait_) {
            if (Enqueue(std::move(element))) {
                return true;
            }
            if (wait_strategy_->EmptyWait()) {
                continue;
            }
            // wait timeout
            break;
        }

        return false;
    }
    bool Dequeue(T* element) {
        uint64_t new_head = 0;
        uint64_t old_head = head_.load(std::memory_order_acquire);
        do {
            new_head = old_head + 1;
            if (new_head == commit_.load(std::memory_order_acquire)) {
                return false;
            }
            *element = pool_[GetIndex(new_head)];
        } while (
            !head_.compare_exchange_weak(old_head, new_head, std::memory_order_acq_rel, std::memory_order_relaxed));
        return true;
    }
    bool WaitDequeue(T* element) {
        while (!break_all_wait_) {
            if (Dequeue(element)) {
                return true;
            }
            if (wait_strategy_->EmptyWait()) {
                continue;
            }
            // wait timeout
            break;
        }

        return false;
    }
    uint64_t Size() { return tail_ - head_ - 1; }
    bool Empty() { return Size() == 0; }
    void SetWaitStrategy(WaitStrategy* WaitStrategy) { wait_strategy_.reset(strategy); }
    void BreakAllWait() {
        break_all_wait_ = true;
        wait_strategy_->BreakAllWait();
    }
    uint64_t Head() { return head_.load(); }
    uint64_t Tail() { return tail_.load(); }
    uint64_t Commit() { return commit_.load(); }

private:
    uint64_t GetIndex(uint64_t num) {
        return num - (num / pool_size_) * pool_size_;  // faster than %
    }

    alignas(CACHELINE_SIZE) std::atomic<uint64_t> head_ = {0};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> tail_ = {1};
    alignas(CACHELINE_SIZE) std::atomic<uint64_t> commit_ = {1};
    // alignas(CACHELINE_SIZE) std::atomic<uint64_t> size_ = {0};
    uint64_t pool_size_ = 0;
    T* pool_ = nullptr;
    std::unique_ptr<WaitStrategy> wait_strategy_ = nullptr;
    volatile bool break_all_wait_ = false;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_BOUNDED_QUEUE_HPP__
