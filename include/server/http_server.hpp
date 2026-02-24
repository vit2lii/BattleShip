#pragma once

#include <cstdint>

#include "server/game_store.hpp"

namespace server
{
  class HttpServer
  {
  public:
    explicit HttpServer(GameStore& store);
    void run(std::uint16_t port);

  private:
    GameStore& m_store;
  };

}  // namespace server
