#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <boost/beast/core/string.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

class Util {
 public:
  // MIME type をファイル名の拡張子から調べる
  static boost::beast::string_view mimeType(boost::beast::string_view path);

  // エラーレスポンスをいい感じに作る便利関数
  static boost::beast::http::response<boost::beast::http::string_body>
  badRequest(
      const boost::beast::http::request<boost::beast::http::string_body>& req,
      boost::beast::string_view why);
  static boost::beast::http::response<boost::beast::http::string_body> notFound(
      const boost::beast::http::request<boost::beast::http::string_body>& req,
      boost::beast::string_view target);
  static boost::beast::http::response<boost::beast::http::string_body>
  serverError(
      const boost::beast::http::request<boost::beast::http::string_body>& req,
      boost::beast::string_view what);
};

#endif  // UTIL_H_INCLUDED
