#ifndef AYAME_WEBSOCKET_SESSION_H_INCLUDED
#define AYAME_WEBSOCKET_SESSION_H_INCLUDED

#include <memory>
#include <string>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/system/error_code.hpp>

#include "ayame_hub.h"
#include "websocket.h"

class AyameWebsocketSession
    : public std::enable_shared_from_this<AyameWebsocketSession> {
  AyameHub* hub_;
  std::unique_ptr<Websocket> ws_;
  boost::beast::multi_buffer sending_buffer_;
  std::string room_id_;
  std::string client_id_;

 public:
  AyameWebsocketSession(boost::asio::ip::tcp::socket socket, AyameHub* hub);
  ~AyameWebsocketSession();
  void run(boost::beast::http::request<boost::beast::http::string_body> req);

 private:
  void doAccept(
      boost::beast::http::request<boost::beast::http::string_body> req);
  void onAccept(boost::system::error_code ec);

  void onRead(boost::system::error_code ec, std::size_t bytes_transferred,
              std::string recv_string);
  };

#endif  // AYAME_WEBSOCKET_SESSION_H_INCLUDED
