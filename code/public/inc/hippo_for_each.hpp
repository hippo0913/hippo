/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_FOR_EACH_HPP__
#define __HIPPO_FOR_EACH_HPP__

#include <type_traits>
#include "hippo_namespace.hpp"
#include "hippo_macro.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

DEFINE_TYPE_TRAIT(HasLess, operator<)  // NOLINT

template <class Value, class End>
typename std::enable_if<HasLess<Value>::value && HasLess<End>::value, bool>::type LessThan(const Value& val,
                                                                                           const End& end) {
    return val < end;
}

template <class Value, class End>
typename std::enable_if<!HasLess<Value>::value || !HasLess<End>::value, bool>::type LessThan(const Value& val,
                                                                                             const End& end) {
    return val != end;
}

#define FOR_EACH(i, begin, end) for (auto i = (true ? (begin) : (end)); apollo::hippo::base::LessThan(i, (end)); ++i)

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_FOR_EACH_HPP__
