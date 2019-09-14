#include "ayame_session.h"

#include <spdlog/spdlog.h>
#include <boost/beast/core/error.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/filesystem/path.hpp>

#ifdef _WIN32
#include <codecvt>
#endif

#include "ayame_websocket_session.h"
#include "util.h"

AyameSession::AyameSession(boost::asio::ip::tcp::socket socket, AyameHub* hub)
    : socket_(std::move(socket)), hub_(hub) {}

void AyameSession::run() { doRead(); }

void AyameSession::doRead() {
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = {};

  // Read a request
  boost::beast::http::async_read(
      socket_, buffer_, req_,
      boost::beast::bind_front_handler(&AyameSession::onRead,
                                       shared_from_this()));
}

void AyameSession::onRead(boost::system::error_code ec,
                          std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  // 接続が切られた
  if (ec == boost::beast::http::error::end_of_stream) {
    return doClose();
  }

  if (ec) {
    SPDLOG_ERROR("read: {}", ec.message());
    return;
  }

  // WebSocket の upgrade リクエスト
  if (req_.target() == "/signaling") {
    if (boost::beast::websocket::is_upgrade(req_)) {
      std::make_shared<AyameWebsocketSession>(std::move(socket_), hub_)
          ->run(std::move(req_));
      return;
    } else {
      sendResponse(Util::badRequest(std::move(req_), "Not upgrade request"));
      return;
    }
  }

  handleRequest();
}

void AyameSession::handleRequest() {
  boost::beast::http::request<boost::beast::http::string_body> req(
      std::move(req_));

  // Make sure we can handle the method
  if (req.method() != boost::beast::http::verb::get &&
      req.method() != boost::beast::http::verb::head)
    return sendResponse(
        Util::badRequest(std::move(req), "Unknown HTTP-method"));

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != boost::beast::string_view::npos)
    return sendResponse(Util::badRequest(req, "Illegal request-target"));

  // Build the path to the requested file
  //boost::filesystem::path path =
  //    boost::filesystem::path(*doc_root_) / std::string(req.target());
  boost::filesystem::path path =
      boost::filesystem::path("sample") / std::string(req.target());

  if (req.target().back() == '/') path.append("index.html");

#ifdef _WIN32
  path.imbue(
      std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>()));
#endif
  // Attempt to open the file
  boost::beast::error_code ec;
  boost::beast::http::file_body::value_type body;
  body.open(path.string().c_str(), boost::beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if (ec == boost::system::errc::no_such_file_or_directory) {
    return sendResponse(Util::notFound(req, req.target()));
  }

  // Handle an unknown error
  if (ec) {
    return sendResponse(Util::serverError(req, ec.message()));
  }

  // Cache the size since we need it after the move
  auto const size = body.size();

  // HEAD リクエスト
  if (req.method() == boost::beast::http::verb::head) {
    boost::beast::http::response<boost::beast::http::empty_body> res{
        boost::beast::http::status::ok, req.version()};
    res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(boost::beast::http::field::content_type,
            Util::mimeType(path.string()));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return sendResponse(std::move(res));
  }

  // GET リクエスト
  boost::beast::http::response<boost::beast::http::file_body> res{
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(boost::beast::http::status::ok, req.version())};
  res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(boost::beast::http::field::content_type,
          Util::mimeType(path.string()));
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return sendResponse(std::move(res));
}

void AyameSession::onWrite(bool close, boost::system::error_code ec,
                           std::size_t bytes_transferred) {
  boost::ignore_unused(bytes_transferred);

  if (ec) {
    SPDLOG_ERROR("write: {}", ec.message());
    return;
  }

  if (close) {
    doClose();
    return;
  }

  res_ = nullptr;

  doRead();
}

void AyameSession::doClose() {
  boost::system::error_code ec;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
}
