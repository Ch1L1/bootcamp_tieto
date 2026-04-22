#include <iostream>
#include "Message.pb.h"

int main() {
    std::cout << "[Client] Starting up..." << std::endl;

    callsim::CallIntent intent;
    intent.set_transaction_id("tx-55512");
    intent.set_best_effort_topic("API Sync");

    auto* caller = intent.mutable_caller();
    caller->set_id("client_01");
    caller->set_display_name("Alice");
    caller->set_client_version("v1.2.0");

    auto* callee = intent.mutable_callee();
    callee->set_id("client_02");
    callee->set_display_name("Bob");

    auto* session = intent.mutable_session();
    session->set_session_id("sess-999");
    session->set_topic("API Sync");

    std::cout << "[Client] Generated CallIntent from " 
              << intent.caller().display_name() << " to " 
              << intent.callee().display_name() 
              << " | Topic: " << intent.best_effort_topic() << std::endl;

    return 0;
}