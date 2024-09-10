/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_OBJECT_POOL_HPP__
#define __HIPPO_OBJECT_POOL_HPP__

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <new>
#include <utility>

#include "hippo_namespace.hpp"
#include "hippo_for_each.hpp"
#include "hippo_macro.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

template <typename T>
class ObjectPool : public std::enable_shared_from_this<ObjectPool<T>> {
public:
    using InitFunc = std::function<void(T *)>;
    using ObjectPoolPtr = std::shared_ptr<ObjectPool<T>>;

    template <typename... Args>
    explicit ObjectPool(uint32_t num_objects, Args &&... args) : num_objects_(num_objects) {
        const size_t size = sizeof(Node);
        object_arena_ = static_cast<char *>(std::calloc(num_objects_, size));
        if (object_arena_ == nullptr) {
            throw std::bad_alloc();
        }

        FOR_EACH(i, 0, num_objects_) {
            T *obj = new (object_arena_ + i * size) T(std::forward<Args>(args)...);
            reinterpret_cast<Node *>(obj)->next = free_head_;
            free_head_ = reinterpret_cast<Node *>(obj);
        }
    }

    template <typename... Args>
    ObjectPool(uint32_t num_objects, InitFunc f, Args &&... args) : num_objects_(num_objects) {
        const size_t size = sizeof(Node);
        object_arena_ = static_cast<char *>(std::calloc(num_objects_, size));
        if (object_arena_ == nullptr) {
            throw std::bad_alloc();
        }

        FOR_EACH(i, 0, num_objects_) {
            T *obj = new (object_arena_ + i * size) T(std::forward<Args>(args)...);
            f(obj);
            reinterpret_cast<Node *>(obj)->next = free_head_;
            free_head_ = reinterpret_cast<Node *>(obj);
        }
    }

    virtual ~ObjectPool() {
        if (object_arena_ != nullptr) {
            const size_t size = sizeof(Node);
            FOR_EACH(i, 0, num_objects_) { reinterpret_cast<Node *>(object_arena_ + i * size)->object.~T(); }
            std::free(object_arena_);
        }
    }

    std::shared_ptr<T> GetObject() {
        if (hippo_unlikely(free_head_ == nullptr)) {
            return nullptr;
        }

        auto self = this->shared_from_this();
        auto obj =
            std::shared_ptr<T>(reinterpret_cast<T *>(free_head_), [self](T *object) { self->ReleaseObject(object); });
        free_head_ = free_head_->next;
        return obj;
    }

private:
    struct Node {
        T object;
        Node *next;
    };

    ObjectPool(ObjectPool &) = delete;
    ObjectPool &operator=(ObjectPool &) = delete;
    void ReleaseObject(T *) {
        if (hippo_unlikely(object == nullptr)) {
            return;
        }

        reinterpret_cast<Node *>(object)->next = free_head_;
        free_head_ = reinterpret_cast<Node *>(object);
    }

    uint32_t num_objects_ = 0U;
    char *object_arena_ = nullptr;
    Node *free_head_ = nullptr;
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_OBJECT_POOL_HPP__
