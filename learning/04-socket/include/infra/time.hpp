#pragma once

#include <chrono>
#include <format>
#include <string>

namespace common {

using namespace std::chrono_literals;

using Nanos = int64_t;

constexpr Nanos MICROS_TO_NANOS = 1000;
constexpr Nanos MILLIS_TO_MICROS = 1000;
constexpr Nanos SEC_TO_MILLIS = 1000;
constexpr Nanos SEC_TO_MICROS = SEC_TO_MILLIS * MILLIS_TO_MICROS;
constexpr Nanos SEC_TO_NANOS = SEC_TO_MICROS * MICROS_TO_NANOS;

inline auto get_now_nanos() noexcept
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

inline auto& get_now_str(std::string* str) noexcept
{
    auto now = std::chrono::system_clock::now();
    *str = std::format("{:%Y-%m-%d %H:%M:%S}", now);
    return *str;
}

} // namespace common