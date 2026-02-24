#include "server/http_server.hpp"

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iostream>
#include <thread>
#include <utility>

#include "server/http_router.hpp"

namespace server
{
  namespace asio = boost::asio;
  namespace beast = boost::beast;
  namespace http = boost::beast::http;
  using tcp = asio::ip::tcp;

  namespace
  {
    void do_session(tcp::socket socket, GameStore& store)
    {
      beast::flat_buffer buffer;

      for (;;)
      {
        http::request<http::string_body> req;
        beast::error_code ec;
        http::read(socket, buffer, req, ec);
        if (ec == http::error::end_of_stream)
        {
          break;
        }
        if (ec)
        {
          break;
        }

        auto res = handle_request(store, std::move(req));
        http::write(socket, res, ec);
        if (ec)
        {
          break;
        }
        if (!res.keep_alive())
        {
          break;
        }
      }

      beast::error_code ec;
      socket.shutdown(tcp::socket::shutdown_send, ec);
    }
  }  // namespace

  HttpServer::HttpServer(GameStore& store) : m_store(store) {}

  void HttpServer::run(std::uint16_t port)
  {
    asio::io_context ioc{ 1 };
    tcp::acceptor acceptor{ ioc, { tcp::v4(), port } };

    std::cout << "Listening on http://0.0.0.0:" << port << "\n";

    for (;;)
    {
      tcp::socket socket{ ioc };
      acceptor.accept(socket);

      std::thread{ [s = std::move(socket), this]() mutable { do_session(std::move(s), m_store); } }.detach();
    }
  }

}  // namespace server
