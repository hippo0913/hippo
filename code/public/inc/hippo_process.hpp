/*
 * Copyright(C): Hippo code, All Rights Reserved
 *
 * Author: Hippo(yinyanxx1028@gmail.com)
 */

#ifndef __HIPPO_PROCESS_HPP__
#define __HIPPO_PROCESS_HPP__

#include <iostream>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "hippo_namespace.hpp"
#include "hippo_singleton.hpp"

NAMESPACE_HIPPO_BEGIN
NAMESPACE_COMMON_BEGIN

class Process {
public:
    Process() = default;
    ~Process() = default;

    std::string getSelfProcessName() {
        char path[PATH_MAX] = {0};
        if (readlink("/proc/self/exe", path, sizeof(path) - 1) <= 0) {
            return 0;
        }

        std::string processName = path;
        size_t pos = processName.find_last_of("\\/");
        processName = processName.substr(pos + 1);
        return processName;
    }
};

#ifndef HIPPO_PROCESS_INST
#define HIPPO_PROCESS_INST (Hippo::Common::GlobalSingleton<Hippo::Common::Process>::Instance())
#endif  // !HIPPO_PROCESS_INST

NAMESPACE_COMMON_END
NAMESPACE_HIPPO_END

#endif  // !__HIPPO_PROCESS_HPP__
