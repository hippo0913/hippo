/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_RW_LOCK_HPP__
#define __HIPPO_RW_LOCK_HPP__

#include <unistd.h>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>

#include "hippo_namespace.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

class AtomicRWLock {
    friend class ReadLockGuard<AtomicRWLock>;
    friend class WriteLockGuard<AtomicRWLock>;

public:
    static const int32_t RW_LOCK_FREE = 0;
    static const int32_t WRITE_EXCLUSIVE = -1;
    static const uint32_t MAX_RETRY_TIMES = 5;
    AtomicRWLock() {}
    explicit AtomicRWLock(bool write_first) : write_first_(write_first) {}

private:
    // all these function only can used by ReadLockGuard/WriteLockGuard;
    void ReadLock() {
        uint32_t retry_times = 0;
        int32_t lock_num = lock_num_.load();
        if (write_first_) {
            do {
                while (lock_num < RW_LOCK_FREE || write_lock_wait_num_.load() > 0) {
                    if (++retry_times == MAX_RETRY_TIMES) {
                        // saving cpu
                        std::this_thread::yield();
                        retry_times = 0;
                    }
                    lock_num = lock_num_.load();
                }
            } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel,
                                                      std::memory_order_relaxed));
        } else {
            do {
                while (lock_num < RW_LOCK_FREE) {
                    if (++retry_times == MAX_RETRY_TIMES) {
                        // saving cpu
                        std::this_thread::yield();
                        retry_times = 0;
                    }
                    lock_num = lock_num_.load();
                }
            } while (!lock_num_.compare_exchange_weak(lock_num, lock_num + 1, std::memory_order_acq_rel,
                                                      std::memory_order_relaxed));
        }
    }
    void WriteLock() {
        int32_t rw_lock_free = RW_LOCK_FREE;
        uint32_t retry_times = 0;
        write_lock_wait_num_.fetch_add(1);
        while (!lock_num_.compare_exchange_weak(rw_lock_free, WRITE_EXCLUSIVE, std::memory_order_acq_rel,
                                                std::memory_order_relaxed)) {
            // rw_lock_free will change after CAS fail, so init agin
            rw_lock_free = RW_LOCK_FREE;
            if (++retry_times == MAX_RETRY_TIMES) {
                // saving cpu
                std::this_thread::yield();
                retry_times = 0;
            }
        }
        write_lock_wait_num_.fetch_sub(1);
    }

    void ReadUnlock() { lock_num_.fetch_sub(1); }
    void WriteUnlock() { lock_num_.fetch_add(1); }

    AtomicRWLock(const AtomicRWLock&) = delete;
    AtomicRWLock& operator=(const AtomicRWLock&) = delete;
    std::atomic<uint32_t> write_lock_wait_num_ = {0};
    std::atomic<int32_t> lock_num_ = {0};
    bool write_first_ = true;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_RW_LOCK_HPP__
