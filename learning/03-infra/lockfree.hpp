#pragma once

#include "macros.hpp"
#include <atomic>
#include <vector>

namespace common {

template<typename T>
class Spsc {
    std::vector<T> data_;
    std::atomic<size_t> next_get_;
    std::atomic<size_t> next_set_;
public:
    explicit Spsc(size_t cap)
    : data_(cap), next_get_(0ULL), next_set_(0ULL) {}

    Spsc(const Spsc&) = delete;
    Spsc(Spsc&&) = delete;
    Spsc& operator=(const Spsc&) = delete;
    Spsc& operator=(Spsc&&) = delete;

    const T* get_next_get() const noexcept
    {
        return (next_get_ == next_set_) ? nullptr : &data_[next_get_];
    }

    void advance_get() noexcept
    {
        next_get_ = (next_get_ + 1) % data_.size();
    }

    T* get_next_set() noexcept
    {
        return &data_[next_set_];
    }

    void advance_set() noexcept
    {
        next_set_ = (next_set_ + 1) % data_.size();
    }
};

} // namespace common