#include "mempool.hpp"
#include <print>

int main()
{
    common::MemPool<int> mp (2);
    auto obj = mp.alloc(10);
    std::println("obj: {}", *obj);
    mp.dealloc(obj);
    mp.dealloc(obj);

    return 0;
}