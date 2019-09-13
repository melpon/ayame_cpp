#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <boost/asio.hpp>

#include "ayame_server.h"

int main(int argc, char* argv[]) {
  CLI::App app("ayame");

  int log_level = spdlog::level::info;

  auto log_level_map = std::vector<std::pair<std::string, int>>(
      {{"trace", (int)spdlog::level::trace},
       {"debug", (int)spdlog::level::debug},
       {"info", (int)spdlog::level::info},
       {"warn", (int)spdlog::level::warn},
       {"error", (int)spdlog::level::err},
       {"critical", (int)spdlog::level::critical},
       {"off", (int)spdlog::level::off}});
  app.add_option("--log-level", log_level, "ログレベル")
      ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case));

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }

  spdlog::set_level((spdlog::level::level_enum)log_level);

  boost::asio::io_context ioc{1};
  const boost::asio::ip::tcp::endpoint endpoint{
      boost::asio::ip::make_address("0.0.0.0"), 3000};
  std::make_shared<AyameServer>(ioc, endpoint)->run();
  ioc.run();
}
