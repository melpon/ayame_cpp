#ifndef AYAME_SERVER_H_INCLUDED
#define AYAME_SERVER_H_INCLUDED

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "ayame_hub.h"

class AyameServer : public std::enable_shared_from_this<AyameServer> {
  boost::asio::io_context& ioc_;
  boost::asio::ip::tcp::acceptor acceptor_;

  AyameHub hub_;

 public:
  AyameServer(boost::asio::io_context& ioc,
              boost::asio::ip::tcp::endpoint endpoint);

  void run();

 private:
  void doAccept();
  void onAccept(boost::system::error_code ec,
                boost::asio::ip::tcp::socket socket);
};

#endif  // AYAME_SERVER_H_INCLUDED
