#pragma once

#include <iostream>
#include <string>

// c++20: replaced by [[likely]] and [[unlikely]]
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

inline auto FATAL(const std::string& msg) noexcept
{
    std::cerr << "FATAL : " << msg << std::endl;
    exit(EXIT_FAILURE);
}

inline auto ASSERT(bool condition, const std::string& msg) noexcept
{
    if (!condition) [[unlikely]] {
        std::cerr << "ASSERT : " << msg << std::endl;
        exit(EXIT_FAILURE);
    }
}