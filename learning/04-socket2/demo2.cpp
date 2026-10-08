#include "infra/time.hpp"
#include "infra/logger.hpp"
#include "tcp_server.hpp"

#include <atomic>

using namespace common;
using namespace std::literals::chrono_literals;

const std::string iface = "lo";
const std::string ip = "127.0.0.1";
const int port = 12345;


int main(int, char **)
{
    Logger logger_("build-linux/demo2.log");

    // ***** setup TCP server in a daemon thread (TODO)

    auto tcp_server_recv_callback = [&](TCPSocket *socket, Nanos rx_time) noexcept {
        INFO(logger_, "TCP server recv: socket %, len %, rx_time %", 
            socket->socket_fd_, socket->next_rcv_valid_index_, rx_time);
        const std::string reply = "TCP server recv msg: " + std::string(socket->inbound_data_.data(), socket->next_rcv_valid_index_);
        socket->next_rcv_valid_index_ = 0;
        socket->send(reply.data(), reply.size());
    };

    auto tcp_server_recv_fin_callback = [&]() noexcept {
        INFO(logger_, "TCP server recv done");
    };

    INFO(logger_, "Creating TCPServer on iface:% port:%", iface, port);
    TCPServer server(logger_);
    server.recv_callback_ = tcp_server_recv_callback;
    server.recv_finished_callback_ = tcp_server_recv_fin_callback;
    server.listen(iface, port);

    std::atomic<bool> stop = false;

    auto* t_svr = common::create_and_start_thread(1, "t_svr", [&]() {
        while (stop) {
            std::this_thread::sleep_for(500ms);
            server.poll();
            server.send_and_recv();
        }
        INFO(logger_, "Stopping server...");
        std::this_thread::sleep_for(1s);
    });

    // ***** setup clients

    std::vector<TCPSocket *> clients(5);

    auto tcp_cli_recv_callback = [&](TCPSocket *socket, Nanos rx_time) noexcept {
		const std::string recv_msg = std::string(socket->inbound_data_.data(), socket->next_rcv_valid_index_);
		socket->next_rcv_valid_index_ = 0;

		INFO(logger_, "TCP client recv, socket:% len:% rx:% msg:%",
					socket->socket_fd_, socket->next_rcv_valid_index_, rx_time, recv_msg);
	};

	for (size_t i = 0; i < clients.size(); ++i) {
		clients[i] = new TCPSocket(logger_);
		clients[i]->recv_callback_ = tcp_cli_recv_callback;

		INFO(logger_, "Connecting TCPClient-[%] on ip:% iface:% port:%", i, ip, iface, port);
		clients[i]->connect(ip, iface, port, false);
		server.poll();
	}

	for (auto itr = 0; itr < 5; ++itr) {
		for (size_t i = 0; i < clients.size(); ++i) {
			const std::string client_msg = "CLIENT-[" + std::to_string(i) + "] : Sending " + std::to_string(itr * 100 + i);
			INFO(logger_, "Sending TCPClient-[%] %", i, client_msg);
			clients[i]->send(client_msg.data(), client_msg.length());
			clients[i]->send_and_recv();

			// std::this_thread::sleep_for(500ms);
			// server.poll();
			// server.send_and_recv();
		}
	}

    stop = true;
    std::this_thread::sleep_for(5s);
    t_svr->join();

	return 0;
}