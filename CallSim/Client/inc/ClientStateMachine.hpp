#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "Message.pb.h"

class ClientStateMachine {
private:
    callsim::ClientState current_state_ = callsim::CLIENT_CONNECTED;
    std::vector<std::string> state_history_ = {"CLIENT_CONNECTED"};

public:
    callsim::ClientState get_current_state() const { return current_state_; }

    void handle_transition(callsim::CallSignal signal, callsim::ClientState target_state) {
        if (current_state_ == callsim::CLIENT_CONNECTED) {
            if (signal == callsim::REGISTERED && target_state == callsim::CLIENT_REGISTERED) {
                transition_to(callsim::CLIENT_REGISTERED, "CLIENT_REGISTERED");
                return;
            }
        }
        throw std::runtime_error("FSM Violation: Invalid client state transition triggered!");
    }

    void print_history() const {
        std::cout << "[Client State History]: ";
        for (size_t i = 0; i < state_history_.size(); ++i) {
            std::cout << state_history_[i] << (i == state_history_.size() - 1 ? "" : " -> ");
        }
        std::cout << "\n";
    }

private:
    void transition_to(callsim::ClientState next_state, const std::string& state_name) {
        current_state_ = next_state;
        state_history_.push_back(state_name);
        print_history();
    }
};