#pragma once

#include "logger.hpp"

#include <string>
#include <sstream>
#include <cstring>
#include <print>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/tcp.h>

namespace common {

// get interface ip
inline std::string get_iface_ip(const std::string& name) noexcept
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

// set socket non-blocking
inline bool set_non_block(int fd) noexcept
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

// disable Nagle's algo for a TCP socket
inline bool set_no_delay(int fd) noexcept
{
    int one = 1;
    return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
}

// set TTL for a non-multicast socket
inline bool set_ttl(int fd, int ttl) noexcept
{
    return (setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
}

// set TTL for a multicast socket
inline bool set_mcast_ttl(int fd, int ttl) noexcept
{
    return (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
}

// enable socket software timestamping
inline bool set_so_timestamp(int fd) noexcept
{
    int one = 1;
    return (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) != -1);
}

// check whether a socket op would block
inline bool would_block() noexcept
{
    return errno == EWOULDBLOCK || errno == EINPROGRESS;
}

struct SocketCfg
{
    std::string iface_;
    std::string ip_;
    int port_ = -1;
    bool is_udp_ = false;
    bool is_listening_ = false;
    bool so_timestamp_ = false;

    inline std::string to_str() const noexcept {
        std::stringstream ss {};
        ss << "SocketCfg[iface:" << iface_
           << " ip:" << ip_
           << " port:" << port_
           << " is_udp:" << is_udp_
           << " is_listening:" << is_listening_
           << " so_timestamp:" << so_timestamp_
           << "]";
        return ss.str();
    }
};

// Represents the maximum number of pending / unaccepted TCP connections.
constexpr int MAX_TCP_CONN_BACKLOG = 1024;

// create socket
[[nodiscard]] inline int create_socket(Logger& logger, const SocketCfg& cfg) noexcept
{
    std::string timestr {};
    const std::string ip = cfg.ip_.empty() ? get_iface_ip(cfg.iface_) : cfg.ip_;
    INFO(logger, "socket cfg %", cfg.to_str());
    
    const int input_flags = (cfg.is_listening_ ? AI_PASSIVE : 0) | AI_NUMERICHOST | AI_NUMERICSERV;
    const int socktype = cfg.is_udp_ ? SOCK_DGRAM : SOCK_STREAM;
    const int protocol = cfg.is_udp_ ? IPPROTO_UDP : IPPROTO_TCP;
    const addrinfo hints {
        input_flags,
        AF_INET,
        socktype,
        protocol,
        0,
        0,
        nullptr,
        nullptr,
    };
    addrinfo *result = nullptr;
    const auto rc = getaddrinfo(ip.c_str(), std::to_string(cfg.port_).c_str(), &hints, &result);
    ASSERT(rc == 0, "getaddrinfo() failed. error:" + std::string(gai_strerror(rc)) + "errno:" + strerror(errno));

    int socket_fd = -1;
    int one = 1;
    for (addrinfo *ai = result; ai; ai = ai->ai_next) {
        ASSERT((socket_fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) != -1, 
            "socket() failed. errno:" + std::string(strerror(errno)));
        ASSERT(set_non_block(socket_fd), "set_non_block() failed. errno:" + std::string(strerror(errno)));

        if (!cfg.is_udp_) {
            ASSERT(set_no_delay(socket_fd), "set_no_delay() failed. errno:" + std::string(strerror(errno)));
        }

        if (cfg.is_listening_) {
            // establish connection to specified address.
            ASSERT(connect(socket_fd, ai->ai_addr, ai->ai_addrlen), "connect() failed. errno:" + std::string(strerror(errno)));
            // bind to the specified port number.
            const sockaddr_in addr{AF_INET, htons(cfg.port_), {htonl(INADDR_ANY)}, {}};
            const sockaddr *bind_addr = cfg.is_udp_ ? reinterpret_cast<const sockaddr*>(&addr) : ai->ai_addr;
            ASSERT(bind(socket_fd, bind_addr, sizeof(addr)), "bind() failed. errno:" + std::string(strerror(errno)));
        } else {
            // allow re-using the address in the call to bind()
            ASSERT(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&one), sizeof(one)) == 0, "setsockopt() SO_REUSEADDR failed. errno:" + std::string(strerror(errno)));
        }

        if (!cfg.is_udp_ && cfg.is_listening_) {
            // listen for incoming TCP connections
            ASSERT(listen(socket_fd, MAX_TCP_CONN_BACKLOG) == 0, "listen() failed. errno:" + std::string(strerror(errno)));
        }

        if (cfg.so_timestamp_) {
            // enable software receval timestamp
            ASSERT(set_so_timestamp(socket_fd), "set_so_timestamp() failed. errno:" + std::string(strerror(errno)));
        }
    }
    return socket_fd;
}

} // namespace common