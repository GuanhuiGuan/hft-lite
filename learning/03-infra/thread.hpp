#pragma once

#include <thread>
#include <print>
#include <string>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include <sys/syscall.h>

using namespace std::chrono_literals;

inline auto set_thread_core([[maybe_unused]] int core_id) -> bool
{
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    return (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) == 0);
#else
    return true;
#endif
}

template<typename F, typename... A>
inline auto create_and_start_thread(int core_id, const std::string& name, F&& func, A&&... args) -> std::thread*
{
    std::atomic<bool> running {false};
    auto body = [&]() {
        std::println("core {} thread {}", core_id, name);
        running = true;
        std::forward<F>(func)(std::forward<A>(args)...);
    };
    auto thread = new std::thread(body);
    return thread;
}