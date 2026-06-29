#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
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
        std::thread([this]() { console_loop(); }).detach();
    }

private:
    void console_loop() {
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input.rfind("/call ", 0) == 0) {
                std::string callee_id = input.substr(6);
                initiate_call(callee_id);
            }
        }
    }

    void initiate_call(const std::string& callee_id) {
        callsim::CallIntent intent;
        intent.mutable_caller()->set_id(client_id_);
        intent.mutable_callee()->set_id(callee_id);
        intent.set_transaction_id("txn_" + std::to_string(std::time(nullptr)));

        std::vector<char> write_buf(intent.ByteSizeLong());
        if (!intent.SerializeToArray(write_buf.data(), write_buf.size())) {
            std::cerr << "Failed to serialize CallIntent.\n";
            return;
        }
        
        auto self(shared_from_this());
        boost::asio::post(socket_.get_executor(), [self, write_buf]() {
            self->send_message(write_buf);
        });

        fsm_.handle_transition(callsim::CALL, callsim::CLIENT_CALLING);
        std::cout << "==> Dialing " << callee_id << "... Waiting for server routing.\n";
    }

    void send_message(const std::vector<char>& write_buf) {
        auto self(shared_from_this());
        
        auto buffer_ptr = std::make_shared<std::vector<char>>(write_buf);
        uint32_t len = buffer_ptr->size();
        
        buffer_ptr->insert(buffer_ptr->begin(), reinterpret_cast<char*>(&len), reinterpret_cast<char*>(&len) + sizeof(len));

        boost::asio::async_write(socket_, boost::asio::buffer(*buffer_ptr),
            [self, buffer_ptr](boost::system::error_code ec, std::size_t) {
                if (ec) std::cerr << "Failed to send message: " << ec.message() << "\n";
            });
    }

    void send_registration() {
        callsim::RegistrationRequest request;
        request.set_transaction_id("tx-" + client_id_);
        
        callsim::Endpoint* endpoint = request.mutable_client();
        endpoint->set_id(client_id_);
        endpoint->set_display_name(client_id_ + " Screen");
        endpoint->set_client_version("1.0.0");

        std::vector<char> write_buf(request.ByteSizeLong());
        if (!request.SerializeToArray(write_buf.data(), write_buf.size())) {
            std::cerr << "Failed to serialize RegistrationRequest.\n";
            return;
        }
        uint32_t len = write_buf.size();

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(&len, sizeof(len)));
        buffers.push_back(boost::asio::buffer(write_buf));

        auto self(shared_from_this());
        boost::asio::async_write(socket_, buffers,
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    std::cout << "[" << self->client_id_ << "] Registration request frame sent.\n";
                    self->read_response_length();
                }
            });
    }

    void read_response_length() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&resp_len_, sizeof(resp_len_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
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
            
            fsm_.handle_transition(callsim::REGISTERED, response.client_state());
            std::cout << "[" << client_id_ << "] Registered state finalized.\n";

            waitForIncomingSignals();
        }
    }

    void waitForIncomingSignals() {
        read_message_length();
    }

    void read_message_length() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&resp_len_, sizeof(resp_len_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->read_message_body();
                } else {
                    std::cout << "\n[" << self->client_id_ << "] Connection lost.\n";
                }
            });
    }

    void read_message_body() {
        auto self(shared_from_this());
        read_buf_.resize(resp_len_);
        boost::asio::async_read(socket_, boost::asio::buffer(read_buf_.data(), resp_len_),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->process_incoming_message();
                    self->read_message_length();
                }
            });
    }

    void process_incoming_message() {
        callsim::CallEvent ring_alert;
        if (ring_alert.ParseFromArray(read_buf_.data(), read_buf_.size())) {
            
            if (ring_alert.signal() == callsim::CALL) {
                fsm_.handle_transition(callsim::CALL, callsim::CLIENT_ANSWERING);
                
                std::cout << "\n\n======================================\n";
                std::cout << " RING! Incoming call from: " << ring_alert.emitter().id() << "\n";
                std::cout << " Type '/answer' to accept or '/reject' to decline.\n";
                std::cout << "======================================\n> ";
                std::cout.flush();
            }
        } else {
            std::cerr << "Failed to parse incoming message.\n";
        }
    }

    tcp::socket socket_;
    std::string client_id_;
    ClientStateMachine fsm_;
    uint32_t resp_len_ = 0;
    std::vector<char> read_buf_;
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