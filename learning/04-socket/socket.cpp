#include "socket_util.hpp"

#include <print>
#include <vector>

int main()
{
    common::Logger logger("build-linux/app.log");
    std::vector<std::string> iface_names {"lo", "eth0"};
    for (const auto& name: iface_names) {
        ERROR(logger, "ip of %: %", name, common::get_iface_ip(name));
    }

    common::SocketCfg sockcfg {
        .iface_="",
        .ip_="",
    };
    int sockfd = common::create_socket(logger, sockcfg);
    std::println("sockfd {}", sockfd);

    return 0;
}