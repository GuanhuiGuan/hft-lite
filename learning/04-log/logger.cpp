#include <print>

#include "logger.hpp"

int main()
{
    std::string str {};
    std::println("epoch: {}, {}", common::get_now_nanos(), common::get_now_str(&str));

    common::Logger logger("build-linux/log/app.log");
    logger.log("starting...\n");
    logger.log("%% % % % % % % % % %\n", 'x', 1, 2l, 3ll, 4u, 5ul, 6ull, 7.7f, 8.8);

    return 0;
}