#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <arpa/inet.h>
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
        boost::asio::async_read(socket_, boost::asio::buffer(&body_length_network_, sizeof(body_length_network_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    self->body_length_ = ntohl(self->body_length_network_);
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
                    if (!self->is_registered_) {
                        self->process_registration();
                    } else {
                        self->process_post_registration_message();
                    }
                }
            });
    }

    void deliver_payload(const std::string& payload) {
        auto self(shared_from_this());
        auto buffer_ptr = std::make_shared<std::vector<char>>(payload.begin(), payload.end());
        uint32_t len_network = htonl(static_cast<uint32_t>(buffer_ptr->size()));
        
        buffer_ptr->insert(buffer_ptr->begin(), 
                           reinterpret_cast<char*>(&len_network), 
                           reinterpret_cast<char*>(&len_network) + sizeof(len_network));

        boost::asio::async_write(socket_, boost::asio::buffer(*buffer_ptr),
            [self, buffer_ptr](boost::system::error_code ec, std::size_t) {
                if (ec) std::cerr << "Failed to route message to client.\n";
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
            is_registered_ = true;
            auto self(shared_from_this());
            fsm_context_->set_network_callback([self](const std::string& payload) {
                self->deliver_payload(payload);
            });

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

    void process_post_registration_message() {
        callsim::CallIntent intent;
        if (intent.ParseFromArray(read_buffer_.data(), read_buffer_.size())) {
            std::string caller = intent.caller().id();
            std::string callee = intent.callee().id();
            
            std::cout << "\n[Server] Routing CALL from " << caller << " to " << callee << "...\n";
            
            auto target_session = registry_->get_client(callee);
            
            if (target_session) {
                callsim::CallEvent ring_alert;
                ring_alert.set_signal(callsim::CALL);
                ring_alert.mutable_emitter()->set_id(caller);
                ring_alert.mutable_receiver()->set_id(callee);
                ring_alert.set_session_id("sess_" + caller + "_" + callee);

                std::string payload;
                if (!ring_alert.SerializeToString(&payload)) {
                    std::cerr << "[Server] Failed to serialize CallEvent!\n";
                    return;
                }
                target_session->deliver(payload);
                std::cout << "[Server] Successfully forwarded RING to " << callee << "!\n";
            } else {
                std::cout << "[Server] Routing Failed: " << callee << " is not registered.\n";
                
                auto caller_session = registry_->get_client(caller);
                if (caller_session) {
                    callsim::CallEvent rejection;
                    rejection.set_signal(callsim::REJECTED);
                    rejection.mutable_emitter()->set_id(callee);
                    rejection.mutable_receiver()->set_id(caller);
                    rejection.set_session_id("sess_" + caller + "_" + callee);
                    
                    std::string payload;
                    if (rejection.SerializeToString(&payload)) {
                        caller_session->deliver(payload);
                        std::cout << "[Server] Notified " << caller << " that " << callee << " is unavailable.\n";
                    }
                }
            }
        }
        read_message_length();
    }

    void send_response(const callsim::RegistrationResponse& response) {
        auto self(shared_from_this());
        write_buffer_.resize(response.ByteSizeLong());
        if (!response.SerializeToArray(write_buffer_.data(), write_buffer_.size())) {
            std::cerr << "[Server] Failed to serialize RegistrationResponse!\n";
            return;
        }

        uint32_t len = write_buffer_.size();
        tx_len_network_ = htonl(len);
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(&tx_len_network_, sizeof(tx_len_network_)));
        buffers.push_back(boost::asio::buffer(write_buffer_));

        boost::asio::async_write(socket_, buffers,
            [self, write_buffer = write_buffer_](boost::system::error_code ec, std::size_t) {
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
    uint32_t body_length_network_ = 0;
    uint32_t tx_len_network_ = 0;
    std::vector<char> read_buffer_;
    std::vector<char> write_buffer_;
    bool is_registered_ = false;
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