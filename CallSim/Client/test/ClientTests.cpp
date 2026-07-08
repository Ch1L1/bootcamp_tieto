#include <gtest/gtest.h>
#include "ClientStateMachine.hpp"

TEST(ClientFSMTests, SuccessfulRegistration) {
    ClientStateMachine fsm;
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_CONNECTED);
}

TEST(ClientFSM, AllowedTransitionSucceeds) {
    ClientStateMachine fsm;
    EXPECT_NO_THROW(fsm.handle_transition(callsim::REGISTERED));
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_REGISTERED);
}

TEST(ClientFSMTests, OutgoingCallSuccess) {
    ClientStateMachine fsm;
    fsm.handle_transition(callsim::REGISTERED);
    fsm.handle_transition(callsim::CALL);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_CALLING);
}

TEST(ClientFSMTests, IncomingCallSuccess) {
    ClientStateMachine fsm;
    fsm.handle_transition(callsim::REGISTERED);

    fsm.handle_transition(callsim::CALL);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_ANSWERING);
}

TEST(ClientFSMTests, InvalidTransitionThrows) {
    ClientStateMachine fsm;
    EXPECT_THROW(fsm.handle_transition(callsim::ANSWER), std::runtime_error);
}