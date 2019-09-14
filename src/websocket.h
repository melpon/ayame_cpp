#ifndef WEBSOCKET_H_INCLUDED
#define WEBSOCKET_H_INCLUDED

#include <functional>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/websocket/stream.hpp>

#ifdef USE_SSL
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/websocket/ssl.hpp>
#endif

// ずっと Read しつつ、書き込みが来たら送信してくれる WebSocket
// サーバ用、クライアント用どちらからでも使える。
// 任意のスレッドから sendText を呼ぶことで書き込みができる。
//
// 接続の確立に関しては、nativeSocket() および nativeSecureSocket() 関数を使って自前で行うこと。
class Websocket {
 public:
  typedef boost::beast::websocket::stream<boost::asio::ip::tcp::socket>
      websocket_t;
#ifdef USE_SSL
  typedef boost::beast::websocket::stream<
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>
      ssl_websocket_t;
#endif
  typedef std::function<void(boost::system::error_code ec,
                             std::size_t bytes_transferred, std::string text)>
      read_callback_t;

 private:
  std::unique_ptr<websocket_t> ws_;
#ifdef USE_SSL
  std::unique_ptr<ssl_websocket_t> wss_;
#else
  std::unique_ptr<websocket_t> wss_;
#endif

  boost::beast::multi_buffer read_buffer_;
  std::vector<boost::beast::flat_buffer> write_buffer_;

 public:
  // 非SSL
  Websocket(boost::asio::io_context& ioc);
#ifdef USE_SSL
  // SSL
  Websocket(boost::asio::io_context& ioc, boost::asio::ssl::context ssl_ctx);
#endif
  // 非SSL + ソケット直接
  Websocket(boost::asio::ip::tcp::socket socket);

  bool isSSL() const;

  websocket_t& nativeSocket();
#ifdef USE_SSL
  ssl_websocket_t& nativeSecureSocket();
#endif

  boost::asio::strand<websocket_t::executor_type>& strand();

 public:
  // Websocket からの読み込みを開始する。
  void startToRead(read_callback_t on_read);

 private:
  void doRead(read_callback_t on_read);
  void onRead(read_callback_t on_read, boost::system::error_code ec,
              std::size_t bytes_transferred);

 public:
  void sendText(std::string text);

 private:
  void doWrite();
  void onWrite(boost::system::error_code ec, std::size_t bytes_transferred);
};

#endif  // WEBSOCKET_H_INCLUDED
