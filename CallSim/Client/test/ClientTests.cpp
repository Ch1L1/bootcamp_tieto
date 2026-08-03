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

    fsm.handle_transition(callsim::ANSWERING);
    EXPECT_EQ(fsm.get_current_state(), callsim::CLIENT_ANSWERING);
}

TEST(ClientFSMTests, InvalidTransitionThrows) {
    ClientStateMachine fsm;
    EXPECT_THROW(fsm.handle_transition(callsim::ANSWER), std::runtime_error);
}

TEST(ClientUIHelpersTests, CallConnectedMessageUsesConsistentFormat) {
    EXPECT_EQ(format_call_connected_message("Bob"), "[CALL CONNECTED] Now talking with Bob.");
    EXPECT_EQ(format_call_connected_message("Lora"), "[CALL CONNECTED] Now talking with Lora.");
}