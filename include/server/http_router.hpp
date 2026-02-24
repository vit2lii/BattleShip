#pragma once

#include <boost/beast/http.hpp>

#include "server/game_store.hpp"

namespace server
{
  namespace http = boost::beast::http;

  http::response<http::string_body> handle_request(GameStore& store, http::request<http::string_body> req);

}  // namespace server
