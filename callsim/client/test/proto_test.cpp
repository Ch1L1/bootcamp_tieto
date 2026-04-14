#include <gtest/gtest.h>
#include "callsim/v1/message.pb.h"

using namespace callsim::v1;

TEST(CallSim, ProtoSanity) {
    RegistrationRequest req;
    req.set_transaction_id("TX-123");
    
    Endpoint* client = req.mutable_client();
    client->set_display_name("Tomas");

    EXPECT_EQ(req.transaction_id(), "TX-123");
    EXPECT_EQ(req.client().display_name(), "Tomas");

    RegistrationResponse res;
    res.set_signal(CALL_SIGNAL_REGISTERED);
    res.set_transaction_id("TX-123");
    res.set_message("Welcome to CallSim");

    EXPECT_EQ(res.signal(), CALL_SIGNAL_REGISTERED);
    EXPECT_EQ(res.message(), "Welcome to CallSim");
}

TEST(CallSim, SerializationCycle) {
    RegistrationResponse original;
    original.set_transaction_id("tx-serial-999");
    original.set_signal(CALL_SIGNAL_ACCEPTED);
    original.set_message("Connection established");

    std::string serialized_data;
    ASSERT_TRUE(original.SerializeToString(&serialized_data));

    std::string network_buffer = serialized_data;

    RegistrationResponse decoded;
    ASSERT_TRUE(decoded.ParseFromString(network_buffer));

    EXPECT_EQ(decoded.transaction_id(), "tx-serial-999");
    EXPECT_EQ(decoded.signal(), CALL_SIGNAL_ACCEPTED);
    EXPECT_EQ(decoded.message(), "Connection established");
}

TEST(CallSim, RepeatedFields) {
    Endpoint ep;
    ep.set_id("client-01");

    ep.add_capabilities("audio");
    ep.add_capabilities("video");
    ep.add_capabilities("encryption");

    EXPECT_EQ(ep.capabilities_size(), 3);
    EXPECT_EQ(ep.capabilities(1), "video");
}