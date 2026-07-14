#include <gtest/gtest.h>
#include "ClientSession.hpp"
#include "ClientRegistry.hpp"

TEST(ServerFSM, InitialStateIsConnected) {
    ClientSession session;
    EXPECT_EQ(session.get_state(), callsim::SERVER_CONNECTED);
}

TEST(ServerRegistry, MultiClientStorageVerification) {
    ClientRegistry registry;
    auto client1 = std::make_shared<ClientSession>();
    auto client2 = std::make_shared<ClientSession>();

    client1->handle_registration("alpha_user");
    client2->handle_registration("beta_user");

    EXPECT_TRUE(registry.register_client(client1));
    EXPECT_TRUE(registry.register_client(client2));
    EXPECT_EQ(registry.total_registered(), 2);
}