#include "time.hpp"
#include "logger.hpp"
#include "tcp_server.hpp"

#include <print>
#include <vector>

int main()
{
    common::Logger logger("build-linux/app.log");
    auto default_svr_recv_callback = [&](common::TCPSocket *socket, common::Nanos rx_time) noexcept
    {
        INFO(logger, "default_svr_recv_callback: socket:%, len:%, rx:%", socket->fd_, socket->next_recv_valid_idx_, rx_time);
        const std::string reply = "TCPServer received msg: " + std::string(socket->recv_buffer_, socket->next_recv_valid_idx_);
        socket->next_recv_valid_idx_ = 0;
        socket->send(reply.data(), reply.size());
    };
    auto default_svr_recv_finished_callback = [&]() noexcept
    {
        INFO(logger, "default_svr_recv_finished_callback");
    };
    auto default_cli_recv_callback = [&](common::TCPSocket *socket, common::Nanos rx_time) noexcept
    {
        const std::string msg = std::string(socket->recv_buffer_, socket->next_recv_valid_idx_);
        socket->next_recv_valid_idx_ = 0;
        INFO(logger, "default_cli_recv_callback: socket:%, len:%, rx:%, msg:%", socket->fd_, msg.size(), rx_time, msg);
    };

    std::cout << "[main]starting" << std::endl;

    const std::string iface = "lo";
    const std::string ip = "127.0.0.1";
    const int port = 33333;
    common::TCPServer server(logger);
    server.recv_callback_ = default_svr_recv_callback;
    server.recv_finished_callback_ = default_svr_recv_finished_callback;
    server.listen(iface, port);

    std::cout << "[main]server established" << std::endl;

    std::vector<common::TCPSocket *> clients (5);
    for (size_t i = 0; i < clients.size(); ++i) {
        clients[i] = new common::TCPSocket(logger);
        clients[i]->recv_callback_ = default_cli_recv_callback;
        std::cout << "[main]connecting client " << i << std::endl;
        INFO(logger, "connecting client[%] on ip:%, iface:%, port:%", i, ip, iface, port);
        clients[i]->connect(ip, iface, port, false);
        server.poll();
    }

    std::cout << "[main]clients connected" << std::endl;

    for (size_t itr = 0; itr < 5; ++itr) {
        for (size_t i = 0; i < clients.size(); ++i) {
            const std::string msg = "client[" + std::to_string(i) + "]: sending " + std::to_string(itr * 1000);
            INFO(logger, "sending client[%] %", i, msg);
            clients[i]->send(msg.data(), msg.size());
            clients[i]->recv_and_send();
            std::this_thread::sleep_for(500ms);
            server.poll();
            server.recv_and_send();
        }
    }
    std::cout << "[main]sent from clients and received by server" << std::endl;

    for (size_t itr = 0; itr < 5; ++itr) {
        for (auto &client : clients) {
            client->recv_and_send();
        }
        server.poll();
        server.recv_and_send();
        std::this_thread::sleep_for(500ms);
    }
    std::cout << "[main]resp from server received by clients" << std::endl;

    for (auto &client : clients) {
        delete client;
    }

    return 0;
}