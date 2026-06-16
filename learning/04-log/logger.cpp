#include "time.hpp"
#include <print>

int main()
{
    std::string str {};
    std::println("epoch: {}, {}", common::get_now_nanos(), common::get_now_str(&str));

    return 0;
}