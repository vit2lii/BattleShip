#pragma once

#include <mutex>
#include <optional>
#include <memory>
#include <string>
#include <unordered_map>

#include "server/game_types.hpp"

#include "project/core/boat.hpp"
#include "project/core/coordinate.hpp"
#include "project/core/placement.hpp"

namespace server
{
  class GameStore
  {
  public:
    CreateGameResult createGame();
    JoinGameResult joinGame(const std::string& gameId);

    AuthContext authenticate(const std::string& gameId, const std::string& authHeader) const;

    void placeShip(const std::string& gameId,
                   int playerIndex,
                   battleship::BoatType type,
                   const battleship::Coordinate& start,
                   battleship::Orientation orientation);

    GameStatus readyUp(const std::string& gameId, int playerIndex);

    ShotOutcome shoot(const std::string& gameId, int playerIndex, const battleship::Coordinate& target);

    std::optional<GameView> getGameView(const std::string& gameId) const;

  private:
    static std::string randomId(std::size_t n);
    static std::string randomToken();

  private:
    mutable std::mutex m_mu;
    std::unordered_map<std::string, std::shared_ptr<GameState>> m_games;
  };

}  // namespace server
