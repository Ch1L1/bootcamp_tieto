#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
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
        std::thread([this]() { console_loop(); }).detach();
    }

private:
    void print_help() {
        std::cout << "\n======================================\n";
        std::cout << " Available Commands:\n";
        std::cout << " /call <client_id>   - Call another client\n";
        std::cout << " /answer             - Accept an incoming call\n";
        std::cout << " /reject             - Decline an incoming call\n";
        std::cout << " /help               - Show this help message\n";
        std::cout << " /exit               - Disconnect\n";
        std::cout << "======================================\n";
    }

    void post_to_io(std::function<void()> action) {
        auto self(shared_from_this());
        boost::asio::post(socket_.get_executor(), [self, action = std::move(action)]() {
            action();
        });
    }

    void console_loop() {
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input.empty()) {
                continue;
            }

            if (input == "/help") {
                print_help();
            } else if (input == "/exit") {
                std::cout << "[" << client_id_ << "] Disconnecting...\n";
                post_to_io([self = shared_from_this()]() {
                    self->disconnect();
                });
                break;
            } else if (input.rfind("/call ", 0) == 0) {
                std::string callee_id = input.substr(6);

                if (callee_id.empty()) {
                    std::cout << "[ERROR] Please provide a client ID. Usage: /call <client_id>\n";
                } else if (callee_id == client_id_) {
                    std::cout << "[ERROR] Cannot call yourself!\n";
                } else {
                    post_to_io([self = shared_from_this(), callee_id]() {
                        self->do_initiate_call(callee_id);
                    });
                }
            } else if (input == "/answer") {
                post_to_io([self = shared_from_this()]() {
                    self->do_answer_call();
                });
            } else if (input == "/reject") {
                post_to_io([self = shared_from_this()]() {
                    self->do_reject_call();
                });
            } else {
                std::cout << "[ERROR] Unknown command: '" << input << "'\n";
                std::cout << "Type '/help' for available commands.\n";
            }
        }
    }

    void do_initiate_call(const std::string& callee_id) {
        if (fsm_.get_current_state() != callsim::CLIENT_REGISTERED) {
            std::cout << "[ERROR] Cannot initiate call until registered.\n";
            return;
        }

        callsim::CallIntent intent;
        intent.mutable_caller()->set_id(client_id_);
        intent.mutable_callee()->set_id(callee_id);
        intent.set_transaction_id("txn_" + std::to_string(std::time(nullptr)));

        std::vector<char> write_buf(intent.ByteSizeLong());
        if (!intent.SerializeToArray(write_buf.data(), write_buf.size())) {
            std::cerr << "Failed to serialize CallIntent.\n";
            return;
        }

        send_message(write_buf);
        fsm_.handle_transition(callsim::CALL);
        pending_callee_ = callee_id;
        std::cout << "==> Dialing " << callee_id << "... Waiting for response.\n";
    }

    void do_answer_call() {
        if (!has_incoming_call_) {
            std::cout << "[ERROR] No incoming call to answer!\n";
            return;
        }

        active_session_id_ = incoming_session_id_;
        active_remote_id_ = incoming_caller_;
        std::cout << "==> Answering call from " << incoming_caller_ << "...\n";
        fsm_.handle_transition(callsim::ACCEPTED);
        has_incoming_call_ = false;

        callsim::CallEvent answer_event;
        answer_event.set_signal(callsim::ACCEPTED);
        answer_event.mutable_emitter()->set_id(client_id_);
        answer_event.mutable_receiver()->set_id(incoming_caller_);
        answer_event.set_session_id(incoming_session_id_);

        std::string payload;
        if (answer_event.SerializeToString(&payload)) {
            send_message(std::vector<char>(payload.begin(), payload.end()));
            std::cout << "[CALL CONNECTED] Talking with " << active_remote_id_ << ".\n";
        }
    }

    void do_reject_call() {
        if (!has_incoming_call_) {
            std::cout << "[ERROR] No incoming call to reject!\n";
            return;
        }

        std::cout << "==> Rejecting call from " << incoming_caller_ << "...\n";
        fsm_.handle_transition(callsim::REJECTED);
        has_incoming_call_ = false;

        callsim::CallEvent reject_event;
        reject_event.set_signal(callsim::REJECTED);
        reject_event.mutable_emitter()->set_id(client_id_);
        reject_event.mutable_receiver()->set_id(incoming_caller_);
        reject_event.set_session_id(incoming_session_id_);
        reject_event.set_context("callee_rejected");

        std::string payload;
        if (reject_event.SerializeToString(&payload)) {
            send_message(std::vector<char>(payload.begin(), payload.end()));
        }
    }

    void disconnect() {
        boost::system::error_code ignored_ec;
        socket_.close(ignored_ec);
    }

    void send_message(const std::vector<char>& write_buf) {
        if (!socket_.is_open()) {
            std::cerr << "Cannot send message: socket is closed.\n";
            return;
        }

        auto copy = std::make_shared<std::vector<char>>(write_buf);
        auto self(shared_from_this());
        boost::asio::post(socket_.get_executor(), [self, copy]() {
            self->do_send_message(copy);
        });
    }

    void do_send_message(std::shared_ptr<std::vector<char>> buffer_ptr) {
        uint32_t len_network = htonl(static_cast<uint32_t>(buffer_ptr->size()));
        buffer_ptr->insert(buffer_ptr->begin(),
            reinterpret_cast<char*>(&len_network),
            reinterpret_cast<char*>(&len_network) + sizeof(len_network));

        boost::asio::async_write(socket_, boost::asio::buffer(*buffer_ptr),
            [self = shared_from_this(), buffer_ptr](boost::system::error_code ec, std::size_t) {
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
            std::cout << "[" << client_id_ << "] " << response.message() << "\n";
            
            if (response.signal() == callsim::REJECTED) {
                std::cout << "[ERROR] Registration rejected. Disconnecting...\n";
                socket_.close();
                return;
            }
            
            fsm_.handle_transition(callsim::REGISTERED);
            std::cout << "[" << client_id_ << "] Registered.\n";
            print_help();
            std::cout << "> ";
            std::cout.flush();

            waitForIncomingSignals();
        }
    }

    void waitForIncomingSignals() {
        read_message_length();
    }

    void read_message_length() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_, boost::asio::buffer(&resp_len_network_, sizeof(resp_len_network_)),
            [self](boost::system::error_code ec, std::size_t) {
                if (!ec) {  
                    self->resp_len_ = ntohl(self->resp_len_network_);
                    
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
                fsm_.handle_transition(callsim::ANSWERING);
                
                has_incoming_call_ = true;
                incoming_caller_ = ring_alert.emitter().id();
                incoming_session_id_ = ring_alert.session_id();
                
                std::cout << "\n\n======================================\n";
                std::cout << " RING! Incoming call from: " << incoming_caller_ << "\n";
                std::cout << " Type '/answer' to accept or '/reject' to decline.\n";
                std::cout << "======================================\n> ";
                std::cout.flush();
            } else if (ring_alert.signal() == callsim::ACCEPTED) {
                if (ring_alert.receiver().id() == client_id_) {
                    active_session_id_ = ring_alert.session_id();
                    active_remote_id_ = ring_alert.emitter().id();
                    fsm_.handle_transition(callsim::ACCEPTED);
                    pending_callee_.clear();
                    std::cout << "\n[CALL CONNECTED] Now talking with " << active_remote_id_ << ".\n";
                    std::cout << "> ";
                    std::cout.flush();
                }
            } else if (ring_alert.signal() == callsim::REJECTED) {
                if (ring_alert.emitter().id() == pending_callee_) {
                    if (ring_alert.context() == "callee_rejected") {
                        std::cout << "\n[CALL REJECTED] " << pending_callee_ << " declined your call.\n";
                    } else {
                        std::cout << "\n[CALL FAILED] " << pending_callee_ << " is unavailable or does not exist.\n";
                    }
                    pending_callee_.clear();
                    if (fsm_.get_current_state() == callsim::CLIENT_CALLING) {
                        fsm_.handle_transition(callsim::REJECTED);
                    }
                    std::cout << "> ";
                    std::cout.flush();
                }
            }
        } else {
            std::cerr << "Failed to parse incoming message.\n";
        }
    }

    tcp::socket socket_;
    std::string client_id_;
    ClientStateMachine fsm_;

    uint32_t tx_len_network_ = 0;
    uint32_t resp_len_network_ = 0;
    uint32_t resp_len_ = 0;
    std::vector<char> read_buf_;
    
    bool has_incoming_call_ = false;
    std::string incoming_caller_;
    std::string incoming_session_id_;
    std::string pending_callee_;
    std::string active_session_id_;
    std::string active_remote_id_;
};

int main(int argc, char* argv[]) {
    std::string client_name = (argc > 1) ? argv[1] : "client_default";

    try {
        std::cout << "============================================\n";
        std::cout << "Client - CallSim\n";
        std::cout << "Client ID: " << client_name << "\n";
        std::cout << "============================================\n\n";
        
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