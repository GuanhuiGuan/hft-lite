#pragma once

#include "tcp_socket.hpp"

namespace common {

struct TCPServer
{
public:
    explicit TCPServer(Logger &logger) : listener_socket_{logger}, logger_{logger} {}

    ~TCPServer() noexcept
    {
        destroy();
    }

    void destroy() noexcept
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
        listener_socket_.destroy();
    }

    void listen(const std::string &iface, int port);

    void poll() noexcept;

    TCPServer() = delete;
    TCPServer(const TCPServer &) = delete;
    TCPServer(TCPServer &&) = delete;
    TCPServer &operator=(const TCPServer &) = delete;
    TCPServer &operator=(TCPServer &&) = delete;

private:

    bool epoll_add(TCPSocket *socket);

    bool epoll_del(TCPSocket *socket);

    void del(TCPSocket *socket);

public:
    constexpr int MAX_EVENTS = 1024;

    int epoll_fd_ = -1; // file descriptor
    TCPSocket listener_socket_;
    epoll_event events_[MAX_EVENTS];
    std::vector<TCPSocket *> sockets_, recv_sockets_, send_sockets_, disconnected_sockets_;
    Logger &logger_;

    std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_ = nullptr;
    std::function<void()> recv_finished_callback_ = nullptr;
};

} // namespace common 