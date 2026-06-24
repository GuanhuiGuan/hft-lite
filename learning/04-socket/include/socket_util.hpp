#pragma once

#include "logger.hpp"

#include <string>
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
std::string get_iface_ip(const std::string& name) noexcept;

// set socket non-blocking
bool set_non_block(int fd) noexcept;

// disable Nagle's algo for a TCP socket
bool set_no_delay(int fd) noexcept;

// set TTL for a non-multicast socket
bool set_ttl(int fd, int ttl) noexcept;

// set TTL for a multicast socket
bool set_mcast_ttl(int fd, int ttl) noexcept;

// enable socket software timestamping
bool set_so_timestamp(int fd) noexcept;

// check whether a socket op would block
bool would_block() noexcept;

struct SocketCfg
{
    std::string iface_;
    std::string ip_;
    int port_ = -1;
    bool is_udp_ = false;
    bool is_listening_ = false;
    bool so_timestamp_ = false;

    // inline std::string to_string() const noexcept {
    //     return std::format(
    //         f"SocketCfg[iface:{iface_} ip:{ip_} port:{port_}, is_udp:{is_udp_}, is_listening:{is_listening_} so_timestamp_:{so_timestamp_}]");
    // }
};

// create socket
// [[nodiscard]] int create_socket(Logger& logger, const SocketCfg& cfg) noexcept;

} // namespace common