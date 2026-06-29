#include <gtest/gtest.h>
#include "ClientStateMachine.hpp"

TEST(ClientFSMTests, SuccessfulRegistration) {
    ClientStateMachine fsm;
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_CONNECTED);
    
    fsm.handle_transition(callsim::REGISTERED, callsim::CLIENT_REGISTERED);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_REGISTERED);
}

TEST(ClientFSMTests, OutgoingCallSuccess) {
    ClientStateMachine fsm;
    fsm.handle_transition(callsim::REGISTERED, callsim::CLIENT_REGISTERED);
    
    fsm.handle_transition(callsim::CALL, callsim::CLIENT_CALLING);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_CALLING);
}

TEST(ClientFSMTests, IncomingCallSuccess) {
    ClientStateMachine fsm;
    fsm.handle_transition(callsim::REGISTERED, callsim::CLIENT_REGISTERED);
    
    fsm.handle_transition(callsim::CALL, callsim::CLIENT_ANSWERING);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_ANSWERING);
}

TEST(ClientFSMTests, InvalidTransitionThrows) {
    ClientStateMachine fsm;
    
    EXPECT_THROW(fsm.handle_transition(callsim::CALL, callsim::CLIENT_CALLING), std::runtime_error);
}