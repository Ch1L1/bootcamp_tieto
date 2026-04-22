#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include "Message.pb.h"

using namespace callsim;

int main() {
    ServerState current_state = SERVER_STATE_UNSPECIFIED;
    std::cout << "[Server] State: SERVER_STATE_UNSPECIFIED (" << current_state << ")\n";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    std::cout << "[Server] Waiting for client to connect on port 8080...\n";

    int client_socket = accept(server_fd, nullptr, nullptr);
    current_state = SERVER_CONNECTED;
    std::cout << "[Server] State Transition -> SERVER_CONNECTED (" << current_state << ")\n";

    char buffer[4096] = {0};

    int bytes_read = read(client_socket, buffer, 4096);
    RegistrationRequest reg_req;
    reg_req.ParseFromArray(buffer, bytes_read);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    RegistrationResponse reg_resp;
    reg_resp.set_signal(REGISTERED);
    reg_resp.set_client_state(CLIENT_REGISTERED);
    reg_resp.set_server_state(SERVER_REGISTERED_IDLE);
    
    current_state = SERVER_REGISTERED_IDLE;
    std::cout << "[Server] State Transition -> SERVER_REGISTERED_IDLE (" << current_state << ")\n";

    std::string resp_str;
    reg_resp.SerializeToString(&resp_str);
    send(client_socket, resp_str.c_str(), resp_str.length(), 0);

    bytes_read = read(client_socket, buffer, 4096);
    CallIntent call_intent;
    call_intent.ParseFromArray(buffer, bytes_read);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    current_state = SERVER_CALLING;
    std::cout << "[Server] State Transition -> SERVER_CALLING (" << current_state << ")\n";

    CallReply call_reply;
    call_reply.set_signal(CALLING);
    call_reply.set_narrative("Ringing the callee...");
    
    std::string reply_str;
    call_reply.SerializeToString(&reply_str);
    send(client_socket, reply_str.c_str(), reply_str.length(), 0);

    std::cout << "[Server] Call is ringing... Press Ctrl+C to terminate.\n";
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(client_socket);
    close(server_fd);
    return 0;
}