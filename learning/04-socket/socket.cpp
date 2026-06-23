#include "socket_util.hpp"

#include <print>
#include <vector>

int main()
{
    std::vector<std::string> iface_names {"lo", "eth0"};
    for (const auto& name: iface_names) {
        std::println("ip of {}: {}", name, common::get_iface_ip(name));
    }

    return 0;
}