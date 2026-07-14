#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>
#include "Message.pb.h"

class ClientSession {
private:
    std::string client_id_;
    callsim::ServerState current_state_ = callsim::SERVER_CONNECTED;
    std::vector<std::string> state_history_ = {"SERVER_CONNECTED"};

public:
    callsim::ServerState get_state() const { return current_state_; }
    std::string get_id() const { return client_id_; }

    void handle_registration(const std::string& id) {
        if (current_state_ != callsim::SERVER_CONNECTED) {
            throw std::runtime_error("FSM Violation: Session not in CONNECTED state.");
        }
        client_id_ = id;
        current_state_ = callsim::SERVER_REGISTERED_IDLE;
        state_history_.push_back("SERVER_REGISTERED_IDLE (ID: " + id + ")");
        print_history();
    }

    void print_history() const {
        std::cout << "[Server Session " << client_id_ << " History]: ";
        for (size_t i = 0; i < state_history_.size(); ++i) {
            std::cout << state_history_[i] << (i == state_history_.size() - 1 ? "" : " -> ");
        }
        std::cout << "\n";
    }
};