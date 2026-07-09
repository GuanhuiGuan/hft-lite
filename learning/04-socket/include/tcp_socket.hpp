#pragma once

#include <functional>
#include "socket_util.hpp"

namespace common {

constexpr size_t TCP_BUFFER_SIZE = 64 * 1024 * 1024;

struct TCPSocket
{
    explicit TCPSocket(Logger& logger) : logger_{logger}
    {
        send_buffer_ = new char[TCP_BUFFER_SIZE];
        recv_buffer_ = new char[TCP_BUFFER_SIZE];
    }

    ~TCPSocket()
    {
        destroy();
        delete[] send_buffer_; send_buffer_ = nullptr;
        delete[] recv_buffer_; recv_buffer_ = nullptr;
    }

    void destroy() noexcept
    {
        close(fd_);
        fd_ = -1;
    }

    int fd_ = -1;
    Logger &logger_;

    char *send_buffer_ = nullptr;
    size_t next_send_valid_idx_ = 0;
    char *recv_buffer_ = nullptr;
    size_t next_recv_valid_idx_ = 0;

    bool send_disconnected_ = false;
    bool recv_disconnected_ = false;

    // socket attributes
    sockaddr_in sock_attr_ {};
    std::function<void(TCPSocket *s, Nanos rx_time)> recv_callback_ = nullptr;

    TCPSocket() = delete;
    TCPSocket(const TCPSocket&) = delete;
    TCPSocket(TCPSocket&&) = delete;
    TCPSocket& operator=(const TCPSocket&) = delete;
    TCPSocket& operator=(TCPSocket&&) = delete;
};

} // namespace common