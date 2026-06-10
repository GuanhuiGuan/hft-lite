#include "lockfree.hpp"
#include "thread.hpp"
#include <print>

int main()
{
    common::Spsc<int> sq (16);
    // std::atomic<bool> stop (false);

    auto producer = [&](int total) -> void
    {
        while (total) {
            auto* set = sq.get_next_set();
            *set = total--;
            std::println("set {}", *set);
            sq.advance_set();
            std::this_thread::sleep_for(0.5s);
        }
        // std::this_thread::sleep_for(1s);
        // stop = true;
    };

    auto consumer = [&]() -> void
    {
        std::this_thread::sleep_for(2s);
        while (sq.size()) {
            auto* get = sq.get_next_get();
            if (get) {
                std::println("got {}", *get);
                sq.advance_get();
            }
            std::this_thread::sleep_for(0.5s);
        }
    };

    auto* tp = common::create_and_start_thread(1, "producer", producer, 24);
    auto* tc = common::create_and_start_thread(2, "consumer", consumer);

    tp->join();
    tc->join();

    return 0;
}