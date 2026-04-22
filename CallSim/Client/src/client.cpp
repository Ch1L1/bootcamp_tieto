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
    ClientState current_state = CLIENT_STATE_UNSPECIFIED;
    std::cout << "[Client] State: CLIENT_STATE_UNSPECIFIED (" << current_state << ")\n";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[Client] Connection Failed!\n";
        return -1;
    }
    
    current_state = CLIENT_CONNECTED;
    std::cout << "[Client] State Transition -> CLIENT_CONNECTED (" << current_state << ")\n";

    char buffer[4096] = {0};

    std::this_thread::sleep_for(std::chrono::seconds(1));

    RegistrationRequest reg_req;
    reg_req.set_transaction_id("tx-1");
    reg_req.mutable_client()->set_display_name("Alice");

    std::string req_str;
    reg_req.SerializeToString(&req_str);
    send(sock, req_str.c_str(), req_str.length(), 0);

    int bytes_read = read(sock, buffer, 4096);
    RegistrationResponse reg_resp;
    reg_resp.ParseFromArray(buffer, bytes_read);

    current_state = reg_resp.client_state();
    std::cout << "[Client] State Transition -> CLIENT_REGISTERED (" << current_state << ")\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));

    CallIntent call_intent;
    call_intent.set_transaction_id("tx-2");
    call_intent.set_best_effort_topic("Urgent API Sync");

    current_state = CLIENT_CALLING;
    std::cout << "[Client] State Transition -> CLIENT_CALLING (" << current_state << ")\n";

    std::string call_str;
    call_intent.SerializeToString(&call_str);
    send(sock, call_str.c_str(), call_str.length(), 0);

    bytes_read = read(sock, buffer, 4096);
    CallReply call_reply;
    call_reply.ParseFromArray(buffer, bytes_read);
    
    std::cout << "[Client] Server replied: " << call_reply.narrative() << "\n";

    std::cout << "[Client] Hearing ringing tone... Press Ctrl+C to hang up.\n";
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(sock);
    return 0;
}