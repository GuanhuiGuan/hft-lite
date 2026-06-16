#pragma once

#include <chrono>
#include <format>
#include <string>

namespace common {

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