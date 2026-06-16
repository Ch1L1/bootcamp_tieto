#include <gtest/gtest.h>
#include "ClientStateMachine.hpp"

TEST(ClientFSM, InitialStateIsConnected) {
    ClientStateMachine fsm;
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_CONNECTED);
}

TEST(ClientFSM, AllowedTransitionSucceeds) {
    ClientStateMachine fsm;
    EXPECT_NO_THROW(fsm.handle_transition(callsim::REGISTERED, callsim::CLIENT_REGISTERED));
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_REGISTERED);
}

TEST(ClientFSM, InvalidTransitionThrowsException) {
    ClientStateMachine fsm;
    EXPECT_THROW(fsm.handle_transition(callsim::ANSWER, callsim::CLIENT_ANSWERING), std::runtime_error);
}