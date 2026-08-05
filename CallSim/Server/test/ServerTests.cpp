#include <gtest/gtest.h>
#include "ClientSession.hpp"
#include "ClientRegistry.hpp"

TEST(ServerSessionTests, RegistrationSuccess) {
    ClientSession session;
    EXPECT_EQ(session.get_state(), callsim::SERVER_CONNECTED);
    
    session.handle_registration("client_alpha");
    
    EXPECT_EQ(session.get_state(), callsim::SERVER_REGISTERED_IDLE);
    EXPECT_EQ(session.get_id(), "client_alpha");
}

TEST(ServerSessionTests, InvalidRegistrationThrows) {
    ClientSession session;
    session.handle_registration("client_alpha");
    
    EXPECT_THROW(session.handle_registration("client_beta"), std::runtime_error);
}

TEST(ServerRegistryTests, AddAndRetrieveClient) {
    ClientRegistry registry;
    auto session = std::make_shared<ClientSession>();
    
    session->handle_registration("client_beta");
    
    EXPECT_TRUE(registry.register_client(session));
    EXPECT_EQ(registry.total_registered(), 1);
    
    auto retrieved_session = registry.get_client("client_beta");
    ASSERT_NE(retrieved_session, nullptr);
    EXPECT_EQ(retrieved_session->get_id(), "client_beta");
    
    auto missing_session = registry.get_client("ghost_client");
    EXPECT_EQ(missing_session, nullptr);
}

TEST(ServerRegistryTests, DuplicateRegistrationIsRejected) {
    ClientRegistry registry;
    auto first_session = std::make_shared<ClientSession>();
    auto second_session = std::make_shared<ClientSession>();

    first_session->handle_registration("client_beta");
    second_session->handle_registration("client_beta");

    EXPECT_TRUE(registry.register_client(first_session));
    EXPECT_FALSE(registry.register_client(second_session));
    EXPECT_EQ(registry.total_registered(), 1);
}

TEST(ServerSessionTests, NetworkCallbackFires) {
    ClientSession session;
    session.handle_registration("client_alpha");
    
    bool callback_fired = false;
    std::string intercepted_payload = "";
    
    session.set_network_callback([&](const std::string& payload) {
        callback_fired = true;
        intercepted_payload = payload;
    });
    
    session.deliver("MOCK_CALL_EVENT_DATA");
    
    EXPECT_TRUE(callback_fired);
    EXPECT_EQ(intercepted_payload, "MOCK_CALL_EVENT_DATA");
}