#include "infra/time.hpp"
#include "infra/logger.hpp"
#include "tcp_server.hpp"

int main(int, char **) {
  using namespace common;

  std::string time_str_;
  Logger logger_("build-linux/app.log");

  auto tcpServerRecvCallback = [&](TCPSocket *socket, Nanos rx_time) noexcept {
    INFO(logger_, "TCPServer::defaultRecvCallback() socket:% len:% rx:%",
                socket->socket_fd_, socket->next_rcv_valid_index_, rx_time);

    const std::string reply = "TCPServer received msg:" + std::string(socket->inbound_data_.data(), socket->next_rcv_valid_index_);
    socket->next_rcv_valid_index_ = 0;

    socket->send(reply.data(), reply.length());
  };

  auto tcpServerRecvFinishedCallback = [&]() noexcept {
    INFO(logger_, "TCPServer::defaultRecvFinishedCallback()");
  };

  auto tcpClientRecvCallback = [&](TCPSocket *socket, Nanos rx_time) noexcept {
    const std::string recv_msg = std::string(socket->inbound_data_.data(), socket->next_rcv_valid_index_);
    socket->next_rcv_valid_index_ = 0;

    INFO(logger_, "TCPSocket::defaultRecvCallback() socket:% len:% rx:% msg:%",
                socket->socket_fd_, socket->next_rcv_valid_index_, rx_time, recv_msg);
  };

  const std::string iface = "lo";
  const std::string ip = "127.0.0.1";
  const int port = 12345;

  INFO(logger_, "Creating TCPServer on iface:% port:%", iface, port);
  TCPServer server(logger_);
  server.recv_callback_ = tcpServerRecvCallback;
  server.recv_finished_callback_ = tcpServerRecvFinishedCallback;
  server.listen(iface, port);

  std::vector<TCPSocket *> clients(5);

  for (size_t i = 0; i < clients.size(); ++i) {
    clients[i] = new TCPSocket(logger_);
    clients[i]->recv_callback_ = tcpClientRecvCallback;

    INFO(logger_, "Connecting TCPClient-[%] on ip:% iface:% port:%", i, ip, iface, port);
    clients[i]->connect(ip, iface, port, false);
    server.poll();
  }

  using namespace std::literals::chrono_literals;

  for (auto itr = 0; itr < 5; ++itr) {
    for (size_t i = 0; i < clients.size(); ++i) {
      const std::string client_msg = "CLIENT-[" + std::to_string(i) + "] : Sending " + std::to_string(itr * 100 + i);
      INFO(logger_, "Sending TCPClient-[%] %", i, client_msg);
      clients[i]->send(client_msg.data(), client_msg.length());
      clients[i]->sendAndRecv();

      std::this_thread::sleep_for(500ms);
      server.poll();
      server.sendAndRecv();
    }
  }

  return 0;
}