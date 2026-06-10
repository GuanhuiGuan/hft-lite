#include "lockfree.hpp"
#include "thread.hpp"
#include <print>

int main()
{
    common::SpscRaw<int> sq (15);

    auto producer = [&](int total) -> void
    {
        while (total) {
            std::println("set {}: {}", total, sq.try_set(total));
            --total;
            std::this_thread::sleep_for(0.5s);
        }
        std::println("producer terminated");
    };

    auto consumer = [&]() -> void
    {
        std::this_thread::sleep_for(2s);
        int x;
        while (sq.size()) {
            x = INT_MIN;
            std::println("try get: {}, {}", sq.try_get(x), x);
            std::this_thread::sleep_for(0.5s);
        }
        std::println("consumer terminated");
    };

    auto* tp = common::create_and_start_thread(1, "producer", producer, 32);
    auto* tc = common::create_and_start_thread(2, "consumer", consumer);

    tp->join();
    tc->join();

    return 0;
}