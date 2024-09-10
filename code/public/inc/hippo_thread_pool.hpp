/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_THREAD_POOL_HPP__
#define __HIPPO_THREAD_POOL_HPP__

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "hippo_namespace.hpp"
#include "hippo_bounded_queue.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_num, std::size_t max_task_num = 1000) : stop_(false) {
        if (!task_queue_.Init(max_task_num, new BlockWaitStrategy())) {
            throw std::runtime_error("Task queue init failed.");
        }
        workers_.reserve(thread_num);
        for (size_t i = 0; i < thread_num; ++i) {
            workers_.emplace_back([this] {
                while (!stop_) {
                    std::function<void()> task;
                    if (task_queue_.WaitDequeue(&task)) {
                        task();
                    }
                }
            });
        }
    };

    template <typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        // don't allow enqueueing after stopping the pool
        if (stop_) {
            return std::future<return_type>();
        }
        task_queue_.Enqueue([task]() { (*task)(); });
        return res;
    }

    ~ThreadPool() {
        if (stop_.exchange(true)) {
            return;
        }
        task_queue_.BreakAllWait();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers_;
    BoundedQueue<std::function<void()>> task_queue_;
    std::atomic_bool stop_;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_THREAD_POOL_HPP__
