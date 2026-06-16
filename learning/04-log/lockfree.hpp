#pragma once

#include "macros.hpp"
#include <atomic>
#include <vector>
#include <new> // for std::hardware_destructive_interference_size

namespace common {

static constexpr size_t cache_line = std::hardware_destructive_interference_size;

template<typename T>
class Spsc {
    std::vector<T> data_;
    alignas(cache_line) std::atomic<size_t> next_get_;
    alignas(cache_line) std::atomic<size_t> next_set_;
    alignas(cache_line) std::atomic<size_t> size_;
public:
    explicit Spsc(size_t cap)
    : data_(cap), next_get_(0ULL), next_set_(0ULL), size_{0ULL} {}

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
        ASSERT(size_ != 0, "Read an invalid elem in " + std::to_string(pthread_self()));
        --size_;
    }

    T* get_next_set() noexcept
    {
        return &data_[next_set_];
    }

    void advance_set() noexcept
    {
        next_set_ = (next_set_ + 1) % data_.size();
        ++size_;
    }

    size_t size() const noexcept
    {
        return size_;
    }
};

template<typename T>
class SpscRaw {
    const size_t cap_;
    T* data_ = nullptr;

    alignas(cache_line) std::atomic<size_t> get_;
    alignas(cache_line) std::atomic<size_t> set_;

    inline size_t slot(size_t i) const noexcept
    {
        return (i & (cap_ - 1));
    }
public:
    SpscRaw(size_t cap) : cap_{cap + 1}, get_{0ULL}, set_{0ULL} 
    {
        ASSERT(cap_ > 0 && (cap_ & (cap_ - 1)) == 0, "cap + 1 should be power of 2");
        data_ = static_cast<T*>(operator new(sizeof(T) * cap_, std::align_val_t(alignof(T))));
    }

    SpscRaw(const SpscRaw&) = delete;
    SpscRaw(SpscRaw&&) = delete;
    SpscRaw& operator=(const SpscRaw&) = delete;
    SpscRaw& operator=(SpscRaw&&) = delete;

    bool try_set(const T& x) noexcept
    {
        auto set = set_.load(std::memory_order_relaxed);
        auto next_set = set + 1;
        if (next_set == get_.load(std::memory_order_acquire)) {
            return false;
        }
        new (&data_[slot(set)]) T(x);
        set_.store(next_set, std::memory_order_release);
        return true;
    }

    bool try_get(T& x) noexcept
    {
        auto get = get_.load(std::memory_order_relaxed);
        if (get == set_.load(std::memory_order_acquire)) {
            return false;
        }
        x = data_[slot(get)];
        get_.store(get + 1, std::memory_order_release);
        return true;
    }

    inline size_t size() const noexcept
    {
        return set_.load(std::memory_order_relaxed) - get_.load(std::memory_order_relaxed);
    }
};

} // namespace common