#include "thread.hpp"

int main() 
{
    auto t = create_and_start_thread(1, "dbg", 
        [](const std::string& arg) {std::println("hello {}", arg);}, std::string("there"));
    std::this_thread::sleep_for(1s);
    t->join();
    delete t;

    return 0;
}