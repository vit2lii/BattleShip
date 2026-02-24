#include "server/game_store.hpp"

#include <random>
#include <stdexcept>
#include <memory>

#include "project/core/boat.hpp"

namespace server
{
  namespace
  {
    std::mt19937& rng()
    {
      static std::mt19937 gen{ std::random_device{}() };
      return gen;
    }
  }  // namespace

  std::string GameStore::randomId(std::size_t n)
  {
    static const char* alphabet = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::uniform_int_distribution<int> d(0, 35);
    std::string s;
    s.reserve(n);
    for (std::size_t i = 0; i < n; ++i)
    {
      s.push_back(alphabet[d(rng())]);
    }
    return s;
  }

  std::string GameStore::randomToken()
  {
    return randomId(2) + "-" + randomId(24);
  }

  CreateGameResult GameStore::createGame()
  {
    std::lock_guard<std::mutex> lk(m_mu);

    const std::string gid = randomId(8);

    auto g = std::make_shared<GameState>();
    g->token[0] = randomToken();
    g->token[1] = randomToken();
    g->status = GameStatus::WaitingForPlayers;

    m_games.emplace(gid, g);

    return CreateGameResult{ .gameId=gid, .playerId=1, .playerToken=g->token[0], .status=g->status };
  }

  JoinGameResult GameStore::joinGame(const std::string& gameId)
  {
    std::lock_guard<std::mutex> lk(m_mu);

    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      throw std::runtime_error("Game not found.");
    }

    auto& g = *it->second;
    if (g.joined[1])
    {
      throw std::runtime_error("Game already has 2 players.");
    }

    g.joined[1] = true;
    g.status = GameStatus::Placing;

    return JoinGameResult{ .gameId=gameId, .playerId=2, .playerToken=g.token[1], .status=g.status };
  }

  AuthContext GameStore::authenticate(const std::string& gameId, const std::string& authHeader) const
  {
    std::lock_guard<std::mutex> lk(m_mu);

    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      return AuthContext{ -1, "" };
    }

    const std::string prefix = "Bearer ";
    if (authHeader.rfind(prefix, 0) != 0)
    {
      return AuthContext{ -1, "" };
    }

    const std::string tok = authHeader.substr(prefix.size());
    const auto& g = *it->second;

    if (tok == g.token[0])
    {
      return AuthContext{ 0, tok };
    }
    if (tok == g.token[1])
    {
      return AuthContext{ 1, tok };
    }
    return AuthContext{ -1, tok };
  }

  void GameStore::placeShip(const std::string& gameId,
                            int playerIndex,
                            battleship::BoatType type,
                            const battleship::Coordinate& start,
                            battleship::Orientation orientation)
  {
    std::lock_guard<std::mutex> lk(m_mu);

    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      throw std::runtime_error("Game not found.");
    }
    auto& g = *it->second;

    if (playerIndex < 0 || playerIndex > 1)
    {
      throw std::runtime_error("Invalid player.");
    }
    if (!g.joined[playerIndex])
    {
      throw std::runtime_error("Player not joined.");
    }
    if (g.status == GameStatus::Finished)
    {
      throw std::runtime_error("Game finished.");
    }

    g.boards[playerIndex].placeStructure(battleship::Boat{ type }, battleship::Placement{ start, orientation });
  }

  GameStatus GameStore::readyUp(const std::string& gameId, int playerIndex)
  {
    std::lock_guard<std::mutex> lk(m_mu);

    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      throw std::runtime_error("Game not found.");
    }
    auto& g = *it->second;

    if (playerIndex < 0 || playerIndex > 1)
    {
      throw std::runtime_error("Invalid player.");
    }

    g.ready[playerIndex] = true;
    g.status = (g.ready[0] && g.ready[1]) ? GameStatus::InProgress : GameStatus::Placing;
    return g.status;
  }

  ShotOutcome GameStore::shoot(const std::string& gameId, int playerIndex, const battleship::Coordinate& target)
  {
    std::lock_guard<std::mutex> lk(m_mu);

    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      throw std::runtime_error("Game not found.");
    }
    auto& g = *it->second;

    if (g.status != GameStatus::InProgress)
    {
      throw std::runtime_error("Game not in progress.");
    }
    if (g.turn != playerIndex)
    {
      throw std::runtime_error("Not your turn.");
    }

    const int enemy = 1 - playerIndex;
    g.boards[enemy].handle_shot(target);

    std::string result = "OK";

    if (g.boards[enemy].allBoatsDestroyed())
    {
      g.status = GameStatus::Finished;
    }
    else
    {
      g.turn = enemy;
    }

    return ShotOutcome{ result, g.turn + 1, g.status };
  }

  std::optional<GameView> GameStore::getGameView(const std::string& gameId) const
  {
    std::lock_guard<std::mutex> lk(m_mu);
    auto it = m_games.find(gameId);
    if (it == m_games.end())
    {
      return std::nullopt;
    }

    const auto& g = *it->second;
    GameView v;
    v.status = g.status;
    v.turn = g.turn;
    v.ready[0] = g.ready[0];
    v.ready[1] = g.ready[1];

    for (int p = 0; p < 2; ++p)
    {
      for (int r = 0; r < battleship::BOARD_SIZE; ++r)
      {
        for (int c = 0; c < battleship::BOARD_SIZE; ++c)
        {
          v.boards[p].cells[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] =
              g.boards[p].getCellView(battleship::Coordinate{ r, c }).cell_state;
        }
      }
    }

    return v;
  }

}  // namespace server
