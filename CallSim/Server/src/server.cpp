#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Message.pb.h"
#include "ClientSession.hpp"
#include "ClientRegistry.hpp"

using boost::asio::ip::tcp;

class NetworkSession : public std::enable_shared_from_this<NetworkSession> {
public:
    NetworkSession(tcp::socket socket, std::shared_ptr<ClientRegistry> registry)
        : socket_(std::move(socket)), registry_(registry), fsm_context_(std::make_shared<ClientSession>()) {}

    void start() {
        read_message_length();
    }

private:
    void read_message_length() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&body_length_, sizeof(body_length_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->read_message_body();
                }
            });
    }

    void read_message_body() {
        auto self(shared_from_this());
        read_buffer_.resize(body_length_);
        boost::asio::async_read(socket_, boost::asio::buffer(read_buffer_.data(), body_length_),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->process_registration();
                }
            });
    }

    void process_registration() {
        callsim::RegistrationRequest request;
        if (!request.ParseFromArray(read_buffer_.data(), read_buffer_.size())) {
            std::cerr << "Failed to parse RegistrationRequest protobuf packet.\n";
            return;
        }

        std::cout << "\n[Server] Received registration request from Client ID: " << request.client().id() << "\n";

        try {
            fsm_context_->handle_registration(request.client().id());
            registry_->register_client(fsm_context_);

            callsim::RegistrationResponse response;
            response.set_signal(callsim::REGISTERED);
            response.set_client_state(callsim::CLIENT_REGISTERED);
            response.set_server_state(callsim::SERVER_REGISTERED_IDLE);
            response.set_message("Registration successful");
            response.set_transaction_id(request.transaction_id());

            send_response(response);
        } catch (const std::exception& e) {
            std::cerr << "FSM Registration Rejected: " << e.what() << "\n";
        }
    }

    void send_response(const callsim::RegistrationResponse& response) {
        auto self(shared_from_this());
        write_buffer_.resize(response.ByteSizeLong());
        response.SerializeToArray(write_buffer_.data(), write_buffer_.size());

        uint32_t len = write_buffer_.size();
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(&len, sizeof(len)));
        buffers.push_back(boost::asio::buffer(write_buffer_));

        boost::asio::async_write(socket_, buffers,
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    std::cout << "[Server] Response sent. Connection safe.\n";
                    
                    self->read_message_length();
                }
            });
    }

    tcp::socket socket_;
    std::shared_ptr<ClientRegistry> registry_;
    std::shared_ptr<ClientSession> fsm_context_;
    uint32_t body_length_ = 0;
    std::vector<char> read_buffer_;
    std::vector<char> write_buffer_;
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), registry_(std::make_shared<ClientRegistry>()) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<NetworkSession>(std::move(socket), registry_)->start();
            }
            do_accept();
        });
    }

    tcp::acceptor acceptor_;
    std::shared_ptr<ClientRegistry> registry_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080);
        std::cout << "CallSim Server listening securely on port 8080...\n";
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}