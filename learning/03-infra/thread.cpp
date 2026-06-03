#include "thread.hpp"

auto sleep_and_say(const std::string& msg, bool sleep) -> void
{
    std::println("starting ...");
    if (sleep) {
        std::this_thread::sleep_for(1s);
    }
    std::println("saying: {}", msg);
    std::println("ending ...");
}

int main() 
{
    auto t1 = create_and_start_thread(1, "dbg", &sleep_and_say, std::string("there"), true);
    auto t2 = create_and_start_thread(2, "dbg", &sleep_and_say, std::string("hello"), false);
    t1->join();
    t2->join();
    delete t1;
    delete t2;

    return 0;
}