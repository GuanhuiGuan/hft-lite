#pragma once

#include <thread>
#include <print>
#include <string>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>

using namespace std::chrono_literals;

namespace common {

inline auto set_thread_core([[maybe_unused]] int core_id) -> bool
{
#ifdef __linux__
    std::println("setting thread affinity {}", core_id);
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    return (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0);
#else
    std::println("skipping thread affinity for mac");
    return true;
#endif
}

template<typename F, typename... A>
inline auto create_and_start_thread(int core_id, const std::string& name, F&& func, A&&... args) -> std::thread*
{
    auto body = [core_id, name, func=std::forward<F>(func), ...args=std::forward<A>(args)]() mutable {
        if (core_id >= 0 && !set_thread_core(core_id)) {
            std::println("failed to set core affinity of thread {} {} to core {}", name, pthread_self(), core_id);
            exit(EXIT_FAILURE);
        }
        std::println("core affinity of thread {} {} has been set to core {}", name, pthread_self(), core_id);
        std::forward<F>(func)(std::forward<A>(args)...);
    };
    auto thread = new std::thread(body);
    return thread;
}

} // namespace common