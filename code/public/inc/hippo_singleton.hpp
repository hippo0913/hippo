/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_SINGLETON__H__
#define __HIPPO_SINGLETON__H__

#include "hippo_namespace.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

// A boost style Signleton implementation
// T must be: no-throw default constructible and no-throw destructible
template <typename T>
struct GlobalSingleton {
public:
    typedef T ObjectType;

    // If, at any point (in user code), GlobalSingleton<T>::Instance()
    //  is called, then the following function is instantiated.
    static ObjectType &Instance() {
        // This is the object that we return a reference to.
        // It is guaranteed to be created before main() begins because of
        //  the next line.
        static ObjectType obj{};

        // The following line does nothing else than force the instantiation
        //  of GlobalSingleton<T>::create_object_, whose constructor is
        //  called before main() begins.
        create_object_.DoNothing();

        return obj;
    }

private:
    struct ObjectCreator {
        // This constructor does nothing more than ensure that Instance()
        //  is called before main() begins, thus creating the static
        //  T object before multithreading race issues can come up.
        ObjectCreator() { GlobalSingleton<T>::Instance(); }
        inline void DoNothing() const {}
    };
    static ObjectCreator create_object_;

    GlobalSingleton();
};

template <typename T>
typename GlobalSingleton<T>::ObjectCreator GlobalSingleton<T>::create_object_;

// A thread local scoped Signleton implementation
// T must be: no-throw default constructible and no-throw destructible
template <typename T>
struct ThreadScopedSingleton {
public:
    typedef T ObjectType;

    static ObjectType &Instance() {
        // wrapper must be a `thread_local` object to make sure
        // there is only one instance in per-thread.
        static thread_local LazyInitializationWrapper wrapper_{};

        return wrapper_.Instance();
    }

private:
    struct LazyInitializationWrapper {
        // for a thread local variable, instances should not
        // be created automatically, so delay the instance
        // creation untill Instance() was invoked.
        LazyInitializationWrapper() : instance_ptr(nullptr) {}

        ~LazyInitializationWrapper() {
            if (instance_ptr) {
                delete instance_ptr;
                instance_ptr = nullptr;
            }
        }

        ObjectType &Instance() {
            if (nullptr == instance_ptr) {
                instance_ptr = new ObjectType{};
            }
            return *instance_ptr;
        }

        ObjectType *instance_ptr;
    };

    ThreadScopedSingleton();
};

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_SINGLETON__H__
