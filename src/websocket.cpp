#include "websocket.h"

#include <utility>

#include <spdlog/spdlog.h>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket/stream.hpp>

#include "util.h"

Websocket::Websocket(boost::asio::io_context& ioc) : ws_(new websocket_t(ioc)) {
  ws_->write_buffer_bytes(8192);
}
#ifdef USE_SSL
Websocket::Websocket(boost::asio::io_context& ioc,
                     boost::asio::ssl::context ssl_ctx)
    : wss_(new ssl_websocket_t(ioc, ssl_ctx)) {
  wss_->write_buffer_bytes(8192);
}
#endif
Websocket::Websocket(boost::asio::ip::tcp::socket socket)
    : ws_(new websocket_t(std::move(socket))) {
  ws_->write_buffer_bytes(8192);
}

bool Websocket::isSSL() const { return ws_ == nullptr; }
Websocket::websocket_t& Websocket::nativeSocket() { return *ws_; }
#ifdef USE_SSL
Websocket::ssl_websocket_t& Websocket::nativeSecureSocket() { return *wss_; }
#endif

void Websocket::startToRead(read_callback_t on_read) { doRead(on_read); }

void Websocket::doRead(read_callback_t on_read) {
  if (isSSL()) {
    wss_->async_read(read_buffer_, boost::beast::bind_front_handler(
                                       &Websocket::onRead, this, on_read));
  } else {
    ws_->async_read(read_buffer_, boost::beast::bind_front_handler(
                                      &Websocket::onRead, this, on_read));
  }
}

void Websocket::onRead(read_callback_t on_read, boost::system::error_code ec,
                       std::size_t bytes_transferred) {
  const auto text = boost::beast::buffers_to_string(read_buffer_.data());
  read_buffer_.consume(read_buffer_.size());

  on_read(ec, bytes_transferred, std::move(text));

  if (ec == boost::asio::error::operation_aborted) {
    return;
  }

  if (ec) {
    SPDLOG_ERROR("onRead: {}", ec.message());
    return;
  }

  doRead(on_read);
}

void Websocket::sendText(std::string text) {
  SPDLOG_DEBUG("send text: {}", text);

  bool empty = write_buffer_.empty();
  boost::beast::flat_buffer buffer;

  const auto n = boost::asio::buffer_copy(buffer.prepare(text.size()),
                                          boost::asio::buffer(text));
  buffer.commit(n);

  write_buffer_.push_back(std::move(buffer));

  if (empty) {
    doWrite();
  }
}

void Websocket::doWrite() {
  auto& buffer = write_buffer_.front();

  if (isSSL()) {
    wss_->text(true);
    wss_->async_write(buffer.data(), boost::beast::bind_front_handler(
                                         &Websocket::onWrite, this));
  } else {
    ws_->text(true);
    ws_->async_write(buffer.data(), boost::beast::bind_front_handler(
                                        &Websocket::onWrite, this));
  }
}

void Websocket::onWrite(boost::system::error_code ec,
                        std::size_t bytes_transferred) {
  if (ec == boost::asio::error::operation_aborted) {
    return;
  }

  if (ec) {
    SPDLOG_ERROR("onWrite: {}", ec.message());
    return;
  }

  write_buffer_.erase(write_buffer_.begin());

  if (!write_buffer_.empty()) {
    doWrite();
  }
}
