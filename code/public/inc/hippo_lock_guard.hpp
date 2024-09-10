/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_LOCK_GUARD_HPP__
#define __HIPPO_LOCK_GUARD_HPP__

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

template <typename RWLock>
class ReadLockGuard {
public:
    explicit ReadLockGuard(RWLock& lock) : rw_lock_(lock) { rw_lock_.ReadLock(); }
    ~ReadLockGuard() { rw_lock_.ReadUnlock(); }

private:
    ReadLockGuard(const ReadLockGuard& other) = delete;
    ReadLockGuard& operator=(const ReadLockGuard& other) = delete;
    RWLock& rw_lock_;
};

template <typename RWLock>
class WriteLockGuard {
public:
    explicit WriteLockGuard(RWLock& lock) : rw_lock_(lock) { rw_lock_.WriteLock(); }

    ~WriteLockGuard() { rw_lock_.WriteUnlock(); }

private:
    WriteLockGuard(const WriteLockGuard& other) = delete;
    WriteLockGuard& operator=(const WriteLockGuard& other) = delete;
    RWLock& rw_lock_;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_LOCK_GUARD_HPP__
