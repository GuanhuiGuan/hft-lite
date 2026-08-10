#include "tcp_server.hpp"

namespace common {

bool TCPServer::epoll_add(TCPSocket *socket)
{
    epoll_event evt{EPOLLET | EPOLLIN, reinterpret_cast<void*>(socket)};
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->fd_, &evt);
}

bool TCPServer::epoll_del(TCPSocket *socket)
{
    return !epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, socket->fd_, nullptr);
}

void TCPServer::del(TCPSocket *socket)
{
    epoll_del(socket);
    std::erase(sockets_, socket);
    std::erase(recv_sockets_, socket);
    std::erase(send_sockets_, socket);
}

void TCPServer::listen(const std::string &iface, int port)
{
    destroy();
    epoll_fd_ = epoll_create(1);
    ASSERT(epoll_fd_ >= 0, "epoll_create() error:" + std::string(std::strerror(errno)));
    ASSERT(listener_socket_.connect("", iface, port, true) >= 0, "listener socket failed to connect. iface:" + iface + 
        " port:" + std::to_string(port) + " errno:" + std::string(std::strerror(errno)));
    ASSERT(epoll_add(&listener_socket_), "epoll_ctl() failed. errno:" + std::string(std::strerror(errno)));
}

void TCPServer::recv_and_send() noexcept
{
    bool recv = false;
    for (const auto& sock : recv_sockets_) {
        recv |= sock->recv_and_send();
    }
    if (recv) {
        recv_finished_callback_();
    }
    for (const auto& sock : send_sockets_) {
        sock->recv_and_send();
    }
}

void TCPServer::poll() noexcept
{
    // unnecessary
    // const int max_events = 1 + sockets_.size(); // 1 is for listener_socket_ for new conn

    for (auto sock : disconnected_sockets_) {
        del(sock);
    }
    disconnected_sockets_.clear();

    const int n = epoll_wait(epoll_fd_, events_, MAX_EVENTS, 0);
    bool has_new_conn = false;
    for (int i = 0; i < n; ++i) {
        epoll_event &evt = events_[i];
        TCPSocket *sock = reinterpret_cast<TCPSocket *>(evt.data.ptr);
        
        if (evt.events & EPOLLIN) {
            if (sock == &listener_socket_) {
                INFO(logger_, "EPOLLIN listener_socket:%", sock->fd_);
                has_new_conn = true;
                continue;
            }
            INFO(logger_, "EPOLLIN socket:%", sock->fd_);
            if (std::find(recv_sockets_.begin(), recv_sockets_.end(), sock) == recv_sockets_.end()) {
                recv_sockets_.emplace_back(sock);
            }
        }

        if (evt.events & EPOLLOUT) {
            INFO(logger_, "EPOLLOUT socket:%", sock->fd_);
            if (std::find(send_sockets_.begin(), send_sockets_.end(), sock) == send_sockets_.end()) {
                send_sockets_.emplace_back(sock);
            }
        }

        if (evt.events & (EPOLLERR | EPOLLHUP)) {
            INFO(logger_, "EPOLLERR socket:%", sock->fd_);
            if (std::find(disconnected_sockets_.begin(), disconnected_sockets_.end(), sock) == disconnected_sockets_.end()) {
                disconnected_sockets_.emplace_back(sock);
            }
        }
    }

    while (has_new_conn) {
        INFO(logger_, "having new conn");
        sockaddr_storage addr;
        socklen_t addr_len = sizeof(addr);
        int fd = accept(listener_socket_.fd_, reinterpret_cast<sockaddr *>(&addr), &addr_len);
        if (fd == -1) break;
        ASSERT(set_non_block(fd) && set_no_delay(fd), "failed to set non-block or no-delay on socket:%" + std::to_string(fd));
        INFO(logger_, "accepted socket:%", fd);

        TCPSocket *sock = new TCPSocket(logger_);
        sock->fd_ = fd;
        sock->recv_callback_ = recv_callback_;
        ASSERT(epoll_add(sock), "unable to add socket. error:" + std::string(std::strerror(errno)));
        INFO(logger_, "EPOLLIN socket:%", sock->fd_);
        
        if (std::find(sockets_.begin(), sockets_.end(), sock) == sockets_.end()) {
            sockets_.emplace_back(sock);
        }
        if (std::find(recv_sockets_.begin(), recv_sockets_.end(), sock) == recv_sockets_.end()) {
            recv_sockets_.emplace_back(sock);
        }
    }
}

} // namespace common