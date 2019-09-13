#ifndef AYAME_SESSION_H_INCLUDED
#define AYAME_SESSION_H_INCLUDED

#include <cstdlib>
#include <functional>
#include <memory>
#include <string>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>

// 1つの HTTP リクエストを処理するためのクラス
class AyameSession : public std::enable_shared_from_this<AyameSession> {
  boost::asio::ip::tcp::socket socket_;
  boost::beast::flat_buffer buffer_;
  boost::beast::http::request<boost::beast::http::string_body> req_;
  std::shared_ptr<void> res_;

 public:
  AyameSession(boost::asio::ip::tcp::socket socket);

  void run();

 private:
  void doRead();
  void onRead(boost::system::error_code ec, std::size_t bytes_transferred);

  void handleRequest();

  template <class Body, class Fields>
  void sendResponse(boost::beast::http::response<Body, Fields> msg) {
    auto sp = std::make_shared<boost::beast::http::response<Body, Fields>>(
        std::move(msg));

    // msg オブジェクトは書き込みが完了するまで生きている必要があるので、
    // メンバに入れてライフタイムを延ばしてやる
    res_ = sp;

    // Write the response
    boost::beast::http::async_write(
        socket_, *sp,
        boost::beast::bind_front_handler(&AyameSession::onWrite,
                                         shared_from_this(), sp->need_eof()));
  }

  void onWrite(bool close, boost::system::error_code ec,
               std::size_t bytes_transferred);
  void doClose();
};

#endif  // AYAME_SESSION_H_INCLUDED
