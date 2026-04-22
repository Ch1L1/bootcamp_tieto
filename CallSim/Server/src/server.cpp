#include <iostream>
#include "Message.pb.h"

int main() {
    std::cout << "[Server] Starting up..." << std::endl;

    callsim::RegistrationRequest incoming_req;
    incoming_req.set_transaction_id("tx-1001");
    
    auto* client_info = incoming_req.mutable_client();
    client_info->set_id("client_01");
    client_info->set_display_name("Alice");

    std::cout << "[Server] Received RegistrationRequest from: " 
              << incoming_req.client().display_name() << std::endl;

    callsim::RegistrationResponse response;
    response.set_signal(callsim::REGISTERED);
    response.set_client_state(callsim::CLIENT_REGISTERED);
    response.set_server_state(callsim::SERVER_REGISTERED_IDLE);
    response.set_transaction_id(incoming_req.transaction_id());
    response.set_message("Registration Successful");

    std::cout << "[Server] Sending RegistrationResponse. " 
              << "New Client State: " << response.client_state() 
              << ", New Server State: " << response.server_state() << std::endl;

    return 0;
}