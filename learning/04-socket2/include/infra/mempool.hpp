#pragma once

#include <vector>

#include "macros.hpp"

namespace common {

template<typename T>
class MemPool final {

    struct Block {
        T obj_;
        bool free_ = true;
    };

    std::vector<Block> store_;
    size_t next_free_idx_ = 0;

public:
    
    explicit MemPool(size_t cap) : store_(cap, {T{}, true})
    {
        ASSERT(cap > 0, "capacity must be greater than 0");
        ASSERT(reinterpret_cast<const Block*>(&store_[0].obj_) == &store_[0], "T obj_ should be the first member");
    }

    MemPool() = delete;
    MemPool(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    template<typename... A>
    T* alloc(A&&... args) noexcept
    {
        Block* block = &store_[next_free_idx_];
        ASSERT(block->free_, "expected free obj at " + std::to_string(next_free_idx_));
        new (&block->obj_) T(std::forward<A>(args)...); // placement new
        block->free_ = false;

        update_next_free_idx();
        return &(block->obj_);
    }

    void dealloc(T* ptr) noexcept
    {
        const size_t idx = (reinterpret_cast<const Block*>(ptr) - &store_[0]);
        ASSERT(idx >= 0 && idx < store_.size(), "ptr does not belong to this mempool");
        ASSERT(!store_[idx].free_, "expected obj in-use at idx " + std::to_string(idx));
        store_[idx].free_ = true;
    }

private:

    void update_next_free_idx() noexcept
    {
        const size_t init_idx = next_free_idx_;
        while (!store_[next_free_idx_].free_) {
            ++next_free_idx_;
            if (next_free_idx_ == store_.size()) [[unlikely]] {
                next_free_idx_ = 0;
            }
            if (next_free_idx_ == init_idx) [[unlikely]] {
                ASSERT(false, "mempool out of memory");
            }
        }
    }
};

} // namespace common