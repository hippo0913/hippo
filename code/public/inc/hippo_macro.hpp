/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_MACRO_HPP__
#define __HIPPO_MACRO_HPP__

#include <type_traits>
#include <cstdlib>
#include <new>

#if __GNUC__ >= 3
#define hippo_likely(x) (__builtin_expect((x), 1))
#define hippo_unlikely(x) (__builtin_expect((x), 0))
#else
#define hippo_likely(x) (x)
#define hippo_unlikely(x) (x)
#endif

#define DEFINE_TYPE_TRAIT(name, func)                          \
    template <typename T>                                      \
    struct name {                                              \
        template <typename Class>                              \
        static constexpr bool Test(decltype(&Class::func) *) { \
            return true;                                       \
        }                                                      \
        template <typename>                                    \
        static constexpr bool Test(...) {                      \
            return false;                                      \
        }                                                      \
                                                               \
        static constexpr bool value = Test<T>(nullptr);        \
    };                                                         \
                                                               \
    template <typename T>                                      \
    constexpr bool name<T>::value;

// void *data = HIPPO_CHECK_MALLOC(size);
#define HIPPO_CHECK_MALLOC(size)                                                                             \
    ({                                                                                                       \
        static_assert(std::is_same<decltype(size), std::size_t>::value, "size must be of type std::size_t"); \
        void *ptr = std::malloc(size);                                                                       \
        if (ptr == nullptr) {                                                                                \
            throw std::bad_alloc();                                                                          \
        }                                                                                                    \
        ptr;                                                                                                 \
    })

// void *data = HIPPO_CHECK_CALLOC(size);
#define HIPPO_CHECK_CALLOC(num, size)                                                                        \
    ({                                                                                                       \
        static_assert(std::is_same<decltype(num), std::size_t>::value, "num must be of type std::size_t");   \
        static_assert(std::is_same<decltype(size), std::size_t>::value, "size must be of type std::size_t"); \
        void *ptr = std::calloc(num, size);                                                                  \
        if (ptr == nullptr) {                                                                                \
            throw std::bad_alloc();                                                                          \
        }                                                                                                    \
        ptr;                                                                                                 \
    })

#endif  // !__HIPPO_MACRO_HPP__
