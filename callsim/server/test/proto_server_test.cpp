#include <gtest/gtest.h>
#include "callsim/v1/message.pb.h"

using namespace callsim::v1;

TEST(ServerProto, RegistrationResponseFlow) {
    RegistrationResponse res;
    res.set_transaction_id("tx-server-123");
    res.set_signal(CALL_SIGNAL_REGISTERED);
    res.set_client_state(CLIENT_STATE_REGISTERED);
    res.set_server_state(SERVER_STATE_REGISTERED_IDLE);

    EXPECT_EQ(res.signal(), CALL_SIGNAL_REGISTERED);
    EXPECT_EQ(res.server_state(), SERVER_STATE_REGISTERED_IDLE);
}

TEST(ServerProto, CallEventBroadcasting) {
    CallEvent event;
    event.set_signal(CALL_SIGNAL_CALLING);
    event.set_session_id("session-alpha-beta");
    
    Endpoint* emitter = event.mutable_emitter();
    emitter->set_id("user-a");
    emitter->set_display_name("Alice");

    EXPECT_STREQ(event.session_id().c_str(), "session-alpha-beta");
    EXPECT_EQ(event.emitter().display_name(), "Alice");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}