#include "tcp_socket.hpp"

namespace common {

int TCPSocket::connect(const std::string& iface, const std::string& ip, int port, bool is_listening)
{
    destroy();
    SocketCfg sockcfg {iface, ip, port, false, is_listening, true};
    fd_ = create_socket(logger_, sockcfg);
    sock_attr_.sin_addr.s_addr = INADDR_ANY;
    sock_attr_.sin_port = htons(port);
    sock_attr_.sin_family = AF_INET;
    return fd_;
}

void TCPSocket::send(const void* data, size_t len) noexcept
{
    if (len > 0) {
        memcpy(recv_buffer_ + next_recv_valid_idx_, data, len);
        next_recv_valid_idx_ += len;
    }
}

bool TCPSocket::recv_and_send() noexcept
{
    // 1) receive
    char ctrl[CMSG_SPACE(sizeof(timeval))];
    auto ctrl_msg = reinterpret_cast<cmsghdr*>(&ctrl); // kernel lvl metadata

    iovec iov {recv_buffer_ + next_recv_valid_idx_, TCP_BUFFER_SIZE - next_recv_valid_idx_};
    msghdr msg {&sock_attr_, sizeof(sock_attr_), &iov, 1, &ctrl, sizeof(ctrl), 0};

    const auto read_size = ::recvmsg(fd_, &msg, MSG_DONTWAIT);
    if (read_size > 0) {
        next_recv_valid_idx_ += read_size;

        Nanos kernel_nanos = 0;
        timeval kernel_time;
        if (ctrl_msg->cmsg_level == SOL_SOCKET && 
            ctrl_msg->cmsg_type == SCM_TIMESTAMP && 
            ctrl_msg->cmsg_len == CMSG_LEN(sizeof(kernel_time))) {
            
            memcpy(&kernel_time, CMSG_DATA(ctrl_msg), sizeof(kernel_time));
            kernel_nanos = kernel_time.tv_sec * SEC_TO_NANOS + kernel_time.tv_usec * MICROS_TO_NANOS;
        }
        const auto user_nanos = get_now_nanos();
        INFO(logger_, "read socket: %, len %, user time: %, kernel time: %, diff: %", fd_, next_recv_valid_idx_, 
            user_nanos, kernel_nanos, user_nanos - kernel_nanos);
        recv_callback_(this, kernel_nanos);
    }
    else if (read_size < 0) {
        const auto err = errno;
        if (err != EAGAIN && err != EWOULDBLOCK) {
            ERROR(logger_, "read error on socket: %, errno: %", fd_, err);
        }
    }

    // 2) send
    if (next_send_valid_idx_ > 0) {
        const auto send_size = ::send(fd_, send_buffer_, next_send_valid_idx_, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (send_size > 0) {
            INFO(logger_, "send socket: %, len: %", fd_, send_size);
            if (send_size < next_send_valid_idx_) {
                size_t unsent_bytes = next_send_valid_idx_ - send_size;
                std::memmove(send_buffer_, send_buffer_ + send_size, unsent_bytes);
                next_send_valid_idx_ = unsent_bytes;
            } else {
                next_send_valid_idx_ = 0;
            }
        } else if (send_size < 0) {
            const auto err = errno;
            if (err != EAGAIN && err != EWOULDBLOCK) {
                ERROR(logger_, "send error on socket: %, errno: %", fd_, err);
                next_send_valid_idx_ = 0;
            }
        }
    }

    return (read_size > 0);
}

} // namespace common