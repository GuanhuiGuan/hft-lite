#pragma once

#include "tcp_socket.hpp"

namespace common {

struct TCPServer
{
public:
    explicit TCPServer(Logger &logger) : listener_socket_{logger}, logger_{logger} {}
public:
    int efd_ = -1; // file descriptor
    TCPSocket listener_socket_;
    epoll_event events_[1024];
    std::vector<TCPSocket *> sockets_, recv_sockets_, send_sockets_, disconnected_sockets_;
    Logger &logger_;

    std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_ = nullptr;
    std::function<void()> recv_finished_callback_ = nullptr;
};

} // namespace common 