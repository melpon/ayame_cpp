#include "ayame_websocket_session.h"

#include <spdlog/spdlog.h>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <nlohmann/json.hpp>

#include "util.h"

using json = nlohmann::json;

AyameWebsocketSession::AyameWebsocketSession(
    boost::asio::ip::tcp::socket socket, AyameHub* hub)
    : ws_(new Websocket(std::move(socket))), hub_(hub) {
  SPDLOG_DEBUG("constructed AyameWebsocketSession");
}

AyameWebsocketSession::~AyameWebsocketSession() {
  hub_->Unregister(room_id_, client_id_);
  SPDLOG_DEBUG("destructed AyameWebsocketSession");
}

void AyameWebsocketSession::run(
    boost::beast::http::request<boost::beast::http::string_body> req) {
  doAccept(std::move(req));
}

void AyameWebsocketSession::doAccept(
    boost::beast::http::request<boost::beast::http::string_body> req) {
  // Accept the websocket handshake
  ws_->nativeSocket().async_accept(
      req, boost::beast::bind_front_handler(&AyameWebsocketSession::onAccept,
                                            shared_from_this()));
}

void AyameWebsocketSession::onAccept(boost::system::error_code ec) {
  if (ec) {
    SPDLOG_ERROR("onAccept: {}", ec.message());
    return;
  }

  // WebSocket での読み込みを開始
  ws_->startToRead(boost::beast::bind_front_handler(
      &AyameWebsocketSession::onRead, shared_from_this()));
}

void AyameWebsocketSession::onRead(boost::system::error_code ec,
                                   std::size_t bytes_transferred,
                                   std::string recv_string) {
  boost::ignore_unused(bytes_transferred);

  if (ec == boost::beast::websocket::error::closed) {
    SPDLOG_INFO("websocket closed");
    return;
  }

  if (ec) {
    SPDLOG_ERROR("onRead: {}", ec.message());
    return;
  }

  SPDLOG_INFO("recv_string={}", recv_string);

  json recv_message;

  try {
    recv_message = json::parse(recv_string);
  } catch (json::parse_error& e) {
    SPDLOG_ERROR("invalid JSON format");
    return;
  }

  if (recv_message.find("type") == recv_message.end()) {
    SPDLOG_ERROR("invalid message: type not found");
    return;
  }

  std::string type = recv_message.at("type");
  if (type == "register") {
    std::string room_id = recv_message.at("roomId");
    std::string client_id = recv_message.at("clientId");
    std::string key = recv_message.value("key", "");
    nlohmann::json metadata =
        recv_message.value("metadata", nlohmann::json::object());
    hub_->Register(room_id, client_id, key, metadata,
                   [this](std::string message) { ws_->sendText(message); });
    room_id_ = room_id;
    client_id_ = client_id;

    ws_->sendText(nlohmann::json(AcceptMessage{"accept"}).dump());
  } else {
    if (room_id_.empty() || client_id_.empty()) {
      SPDLOG_ERROR("does not registered");
      return;
    }
    hub_->Broadcast(room_id_, client_id_, recv_string);
  }

  //if (type == "offer")
  //{
  //    std::string sdp;
  //    try
  //    {
  //        sdp = recv_message["sdp"];
  //    }
  //    catch (json::type_error &e)
  //    {
  //        return;
  //    }

  //    auto send = std::bind(
  //        [](AyameWebsocketSession* session, std::string str) {
  //            session->ws_->sendText(str);
  //        },
  //        this,
  //        std::placeholders::_1);
  //    connection_ = std::make_shared<P2PConnection>(rtc_manager_, send);
  //    std::shared_ptr<RTCConnection> rtc_conn = connection_->getRTCConnection();
  //    rtc_conn->setOffer(sdp);
  //}
  //else if (type == "answer")
  //{
  //    std::shared_ptr<P2PConnection> p2p_conn = connection_;
  //    if (!p2p_conn)
  //    {
  //        return;
  //    }
  //    std::string sdp;
  //    try
  //    {
  //        sdp = recv_message["sdp"];
  //    }
  //    catch (json::type_error &e)
  //    {
  //        return;
  //    }
  //    std::shared_ptr<RTCConnection> rtc_conn = p2p_conn->getRTCConnection();
  //    rtc_conn->setAnswer(sdp);
  //}
  //else if (type == "candidate")
  //{
  //    std::shared_ptr<P2PConnection> p2p_conn = connection_;
  //    if (!p2p_conn)
  //    {
  //        return;
  //    }
  //    int sdp_mlineindex = 0;
  //    std::string sdp_mid, candidate;
  //    try
  //    {
  //        json ice = recv_message["ice"];
  //        sdp_mid = ice["sdpMid"];
  //        sdp_mlineindex = ice["sdpMLineIndex"];
  //        candidate = ice["candidate"];
  //    }
  //    catch (json::type_error &e)
  //    {
  //        return;
  //    }
  //    std::shared_ptr<RTCConnection> rtc_conn = p2p_conn->getRTCConnection();
  //    rtc_conn->addIceCandidate(sdp_mid, sdp_mlineindex, candidate);
  //}
  //else if (type == "close")
  //{
  //    connection_ = nullptr;
  //}
  //else
  //{
  //    return;
  //}
}
