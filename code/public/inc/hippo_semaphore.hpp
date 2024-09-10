/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_SEMAPHORE_HPP__
#define __HIPPO_SEMAPHORE_HPP__

#include <iostream>
#include <semaphore.h>
#include <cassert>

#include "hippo_namespace.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

class Semaphore {
public:
    explicit Semaphore(unsigned int init_cout = 0) {
        int rc = sem_init(&sem_, 0, init_cout);
        assert(rc == 0);
    }
    ~Semaphore() { sem_destroy(&sem_); }

    bool wait() {
        // http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
        int rc = 0;
        do {
            rc = sem_wait(&sem_);
        } while (rc == -1 && errno == EINTR);
        return rc == 0;
    }

    bool try_wait() {
        int rc = 0;
        do {
            rc = sem_trywait(&sem_);
        } while (rc == -1 && errno == EINTR);
        return rc == 0;
    }

    bool timed_wait(std::uint64_t usecs) {
        struct timespec ts;
        constexpr int usecs_in_1_sec = 1000000;
        constexpr int nsecs_in_1_sec = 1000000000;
#ifdef HPC_CONFIG_SEM_MONOTONIC_CLOCK
        clock_gettime(CLOCK_MONOTONIC, &ts);
#else
        clock_gettime(CLOCK_REALTIME, &ts);
#endif
        ts.tv_sec += (time_t)(usecs / usecs_in_1_sec);
        ts.tv_nsec += (long)(usecs % usecs_in_1_sec) * 1000;
        // sem_timedwait bombs if you have more than 1e9 in tv_nsec
        // so we have to clean things up before passing it in
        if (ts.tv_nsec >= nsecs_in_1_sec) {
            ts.tv_nsec -= nsecs_in_1_sec;
            ++ts.tv_sec;
        }

        int rc = 0;
        do {
#ifdef HPC_CONFIG_SEM_MONOTONIC_CLOCK
            rc = sem_clockwait(&sem_, CLOCK_MONOTONIC, &ts);
#else
            rc = sem_timedwait(&sem_, &ts);
#endif
        } while (rc == -1 && errno == EINTR);
        return rc == 0;
    }

    void signal() {
        while (sem_post(&sem_) == -1)
            ;
    }

    void signal(int count) {
        while (count-- > 0) {
            while (sem_post(&sem_) == -1)
                ;
        }
    }

private:
    sem_t sem_;

    Semaphore(const Semaphore& other) = delete;
    Semaphore& operator=(const Semaphore& other) = delete;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_SEMAPHORE_HPP__
