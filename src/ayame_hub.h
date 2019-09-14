#ifndef AYAME_HUB_H_INCLUDED
#define AYAME_HUB_H_INCLUDED

#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

struct AcceptMessage {
  std::string type;
  std::vector<nlohmann::json> ice_servers;
};
static void to_json(nlohmann::json& j, const AcceptMessage& p) {
  j["type"] = p.type;
  if (!p.ice_servers.empty()) {
    j["iceServers"] = p.ice_servers;
  }
}
static void from_json(const nlohmann::json& j, AcceptMessage& p) {
  j.at("type").get_to(p.type);
  if (j.find("iceServers") != j.end()) {
    j.at("iceServers").get_to(p.ice_servers);
  }
}

struct RejectMessage {
  std::string type;
  std::string reason;
};
static void to_json(nlohmann::json& j, const RejectMessage& p) {
  j["type"] = p.type;
  j["reason"] = p.reason;
}
static void from_json(const nlohmann::json& j, RejectMessage& p) {
  j.at("type").get_to(p.type);
  j.at("reason").get_to(p.reason);
}

struct AcceptMetadataMessage {
  std::string type;
  nlohmann::json metadata;
  std::vector<nlohmann::json> ice_servers;
};
static void to_json(nlohmann::json& j, const AcceptMetadataMessage& p) {
  j["type"] = p.type;
  j["metadata"] = p.metadata;
  if (!p.ice_servers.empty()) {
    j["iceServers"] = p.ice_servers;
  }
}
static void from_json(const nlohmann::json& j, AcceptMetadataMessage& p) {
  j.at("type").get_to(p.type);
  p.metadata = j.at("authzMetadata");
  if (j.find("iceServers") != j.end()) {
    j.at("iceServers").get_to(p.ice_servers);
  }
}

class AyameHub {
  struct Client {
    std::string room_id;
    std::string client_id;
    std::string key;
    nlohmann::json metadata;
    std::function<void(std::string)> send;
  };
  std::map<std::string, std::vector<Client>> rooms_;

 public:
  void Register(std::string room_id, std::string client_id, std::string key,
                nlohmann::json metadata,
                std::function<void(std::string)> send) {
    Client client{std::move(room_id), std::move(client_id), std::move(key),
                  std::move(metadata), std::move(send)};
    rooms_[client.room_id].push_back(std::move(client));
    SPDLOG_DEBUG("Register room_id={}, client_id={}", client.room_id,
                 client.client_id);
  }
  void Unregister(std::string room_id, std::string client_id) {
    SPDLOG_DEBUG("Unregister room_id={}, client_id={}", room_id, client_id);
    auto& clients = rooms_[room_id];
    clients.erase(std::remove_if(clients.begin(), clients.end(),
                                 [&client_id](const auto& c) {
                                   return c.client_id == client_id;
                                 }),
                  clients.end());
  }
  void Broadcast(std::string room_id, std::string sender_client_id,
                 std::string message) {
    const auto& clients = rooms_[room_id];
    for (const auto& client : clients) {
      if (client.client_id == sender_client_id) {
        continue;
      }
      SPDLOG_DEBUG("Broadcast room_id={}, client_id={}, message={}",
                   client.room_id, client.client_id, message);
      client.send(message);
    }
  }
};

#endif  // AYAME_HUB_H_INCLUDED
