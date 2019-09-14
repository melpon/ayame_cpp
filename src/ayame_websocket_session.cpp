#include "ayame_websocket_session.h"

#include <spdlog/spdlog.h>
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <nlohmann/json.hpp>

#include "util.h"

using json = nlohmann::json;

AyameWebsocketSession::AyameWebsocketSession(
    boost::asio::ip::tcp::socket socket)
    : ws_(new Websocket(std::move(socket))) {
  SPDLOG_DEBUG("constructed AyameWebsocketSession");
}

AyameWebsocketSession::~AyameWebsocketSession() {
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

  json recv_message;

  SPDLOG_INFO("recv_string={}", recv_string);

  //try
  //{
  //    recv_message = json::parse(recv_string);
  //}
  //catch (json::parse_error &e)
  //{
  //    return;
  //}

  //std::string type;
  //try
  //{
  //  type = recv_message["type"];
  //}
  //catch (json::type_error &e)
  //{
  //    return;
  //}

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
