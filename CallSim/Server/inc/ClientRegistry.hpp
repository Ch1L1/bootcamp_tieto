#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include "ClientSession.hpp"

class ClientRegistry {
private:
    std::unordered_map<std::string, std::shared_ptr<ClientSession>> active_clients_;

public:
    bool register_client(const std::shared_ptr<ClientSession>& session) {
        if (session->get_id().empty() || session->get_state() != callsim::SERVER_REGISTERED_IDLE) {
            return false;
        }
        // Check if client ID already registered
        if (active_clients_.find(session->get_id()) != active_clients_.end()) {
            return false;  // Duplicate ID
        }
        active_clients_[session->get_id()] = session;
        return true;
    }

    std::shared_ptr<ClientSession> get_client(const std::string& id) {
        auto it = active_clients_.find(id);
        if (it != active_clients_.end()) {
            return it->second;
        }
        return nullptr;
    }

    size_t total_registered() const { return active_clients_.size(); }
};