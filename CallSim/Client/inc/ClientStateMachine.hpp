#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include "Message.pb.h"

class ClientStateMachine;

class ClientState {
public:
    virtual ~ClientState() = default;
    virtual void handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) = 0;
    virtual callsim::ClientState get_enum_state() const = 0;
    virtual std::string get_name() const = 0;
};

class ClientStateMachine {
private:
    std::shared_ptr<ClientState> current_state_;
    std::vector<std::string> state_history_;

public:
    ClientStateMachine();

    callsim::ClientState get_current_state() const {
        return current_state_->get_enum_state();
    }

    void set_state(std::shared_ptr<ClientState> new_state) {
        current_state_ = new_state;
        state_history_.push_back(current_state_->get_name());
        print_history();
    }

    void handle_transition(callsim::CallSignal signal) {
        current_state_->handle_signal(signal, *this);
    }

    void print_history() const {
        std::cout << "[Client State History]: ";
        for (size_t i = 0; i < state_history_.size(); ++i) {
            std::cout << state_history_[i] << (i == state_history_.size() - 1 ? "" : " -> ");
        }
        std::cout << "\n";
    }
};

class StateCalling;
class StateAnswering;

class StateConnected : public ClientState {
public:
    callsim::ClientState get_enum_state() const override { return callsim::CLIENT_CONNECTED; }
    std::string get_name() const override { return "CLIENT_CONNECTED"; }
    void handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) override;
};

class StateRegistered : public ClientState {
public:
    callsim::ClientState get_enum_state() const override { return callsim::CLIENT_REGISTERED; }
    std::string get_name() const override { return "CLIENT_REGISTERED"; }
    void handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) override;
};

class StateCalling : public ClientState {
public:
    callsim::ClientState get_enum_state() const override { return callsim::CLIENT_CALLING; }
    std::string get_name() const override { return "CLIENT_CALLING"; }
    void handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) override {
        throw std::runtime_error("FSM Violation: Signal handling in CALLING state not implemented yet!");
    }
};

class StateAnswering : public ClientState {
public:
    callsim::ClientState get_enum_state() const override { return callsim::CLIENT_ANSWERING; }
    std::string get_name() const override { return "CLIENT_ANSWERING"; }
    void handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) override {
        throw std::runtime_error("FSM Violation: Signal handling in ANSWERING state not implemented yet!");
    }
};

inline ClientStateMachine::ClientStateMachine() {
    current_state_ = std::make_shared<StateConnected>();
    state_history_.push_back(current_state_->get_name());
}

inline void StateConnected::handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) {
    if (signal == callsim::REGISTERED) {
        fsm.set_state(std::make_shared<StateRegistered>());
    } else {
        throw std::runtime_error("FSM Violation: Invalid signal for CONNECTED state!");
    }
}

inline void StateRegistered::handle_signal(callsim::CallSignal signal, ClientStateMachine& fsm) {
    if (signal == callsim::CALL) {
        fsm.set_state(std::make_shared<StateCalling>());
    } else if (signal == callsim::ANSWERING) {
        fsm.set_state(std::make_shared<StateAnswering>());
    } else {
        throw std::runtime_error("FSM Violation: Invalid signal for REGISTERED state!");
    }
}