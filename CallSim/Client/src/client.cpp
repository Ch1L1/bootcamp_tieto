#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include "Message.pb.h"
#include "ClientStateMachine.hpp"

using boost::asio::ip::tcp;

class PersistentClient : public std::enable_shared_from_this<PersistentClient> {
public:
    PersistentClient(boost::asio::io_context& io_context, const std::string& client_id)
        : socket_(io_context), client_id_(client_id) {}

    void start(tcp::resolver::results_type endpoints) {
        boost::asio::connect(socket_, endpoints);
        std::cout << "[" << client_id_ << "] Connected to server successfully.\n";
        send_registration();
    }

private:
    void send_registration() {
        callsim::RegistrationRequest request;
        request.set_transaction_id("tx-" + client_id_);
        
        callsim::Endpoint* endpoint = request.mutable_client();
        endpoint->set_id(client_id_);
        endpoint->set_display_name(client_id_ + " Screen");
        endpoint->set_client_version("1.0.0");

        std::vector<char> write_buf(request.ByteSizeLong());
        if (!request.SerializeToArray(write_buf.data(), write_buf.size())) {
            std::cerr << "[" << client_id_ << "] Failed to serialize RegistrationRequest!\n";
            return;
        }
        uint32_t len = write_buf.size();

        tx_len_network_ = htonl(len);

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(&tx_len_network_, sizeof(tx_len_network_)));
        buffers.push_back(boost::asio::buffer(write_buf));

        auto self(shared_from_this());
        boost::asio::async_write(socket_, buffers,
            [self, write_buf](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    std::cout << "[" << self->client_id_ << "] Registration request frame sent.\n";
                    self->read_response_length();
                }
            });
    }

    void read_response_length() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&resp_len_network_, sizeof(resp_len_network_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->resp_len_ = ntohl(self->resp_len_network_);
                    self->read_response_body();
                }
            });
    }

    void read_response_body() {
        auto self(shared_from_this());
        read_buf_.resize(resp_len_);
        boost::asio::async_read(socket_, boost::asio::buffer(read_buf_.data(), resp_len_),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->process_response();
                }
            });
    }

    void process_response() {
        callsim::RegistrationResponse response;
        if (response.ParseFromArray(read_buf_.data(), read_buf_.size())) {
            std::cout << "[" << client_id_ << "] Received response: " << response.message() << "\n";
            
            fsm_.handle_transition(callsim::REGISTERED);
            std::cout << "[" << client_id_ << "] Registered state finalized.\n";

            waitForIncomingSignals();
        }
    }

    void waitForIncomingSignals() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&dummy_byte_, 1),
            [self](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << "[" << self->client_id_ << "] Connection severed by remote end.\n";
                } else {
                    self->waitForIncomingSignals();
                }
            });
    }

    tcp::socket socket_;
    std::string client_id_;
    ClientStateMachine fsm_;

    uint32_t tx_len_network_ = 0;
    uint32_t resp_len_network_ = 0;
    uint32_t resp_len_ = 0;
    std::vector<char> read_buf_;
    char dummy_byte_;
};

int main(int argc, char* argv[]) {
    std::string client_name = (argc > 1) ? argv[1] : "client_default";

    try {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");

        auto client = std::make_shared<PersistentClient>(io_context, client_name);
        client->start(endpoints);

        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Client Error: " << e.what() << "\n";
    }
    return 0;
}