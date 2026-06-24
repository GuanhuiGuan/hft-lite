#include "socket_util.hpp"

namespace common {

std::string get_iface_ip(const std::string& name) noexcept
{
    char buf[NI_MAXHOST] = {'\0'};
    ifaddrs *interfaces = nullptr;
    if (getifaddrs(&interfaces) != -1) {
        for (ifaddrs *ifa = interfaces; ifa; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && ifa->ifa_name == name) {
                getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), // socket address (internet (ipv4))
                    buf, sizeof(buf), // host
                    NULL, 0, // server (port)
                    NI_NUMERICHOST); // flags (return numeric values instead of domain name with dns lookup)
                break;
            }
        }
        freeifaddrs(interfaces);
    }
    return buf;
}

bool set_non_block(int fd) noexcept
{
    const auto flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    if (flags & O_NONBLOCK) {
        return true;
    }
    return (fcntl(fd, F_SETFL, (flags & O_NONBLOCK)) != -1);
}

bool set_no_delay(int fd) noexcept
{
    int one = 1;
    return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
}

bool set_ttl(int fd, int ttl) noexcept
{
    return (setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
}

bool set_mcast_ttl(int fd, int ttl) noexcept
{
    return (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
}

bool set_so_timestamp(int fd) noexcept
{
    int one = 1;
    return (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
}

bool would_block() noexcept
{
    return errno == EWOULDBLOCK || errno == EINPROGRESS;
}

// int create_socket(Logger& logger, const SocketCfg& cfg) noexcept
// {
//     // TODO
//     return 0;
// }

} // namespace common