#include "socket_util.hpp"

namespace common {

std::string get_iface_ip(const std::string& name)
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

} // namespace common