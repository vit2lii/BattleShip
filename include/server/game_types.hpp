#pragma once

#include <array>
#include <optional>
#include <string>

#include "project/core/board.hpp"

namespace server
{
  enum class GameStatus
  {
    WaitingForPlayers,
    Placing,
    InProgress,
    Finished
  };

  inline const char* to_cstr(GameStatus s) noexcept
  {
    switch (s)
    {
      case GameStatus::WaitingForPlayers: return "waiting_for_players";
      case GameStatus::Placing: return "placing";
      case GameStatus::InProgress: return "in_progress";
      case GameStatus::Finished: return "finished";
    }
    return "unknown";
  }

  struct CreateGameResult
  {
    std::string gameId;
    int playerId{};  // 1 or 2
    std::string playerToken;
    GameStatus status{ GameStatus::WaitingForPlayers };
  };

  struct JoinGameResult
  {
    std::string gameId;
    int playerId{};
    std::string playerToken;
    GameStatus status{ GameStatus::WaitingForPlayers };
  };

  struct ShotOutcome
  {
    std::string result;  // "MISS"/"HIT"/"SUNK" or "OK"
    int nextTurnPlayerId{};
    GameStatus status{ GameStatus::WaitingForPlayers };
  };

  struct GameState
  {
    battleship::Board boards[2];
    bool joined[2]{ true, false };
    bool ready[2]{ false, false };
    int turn{ 0 };  // 0 => player1, 1 => player2
    std::array<std::string, 2> token;
    GameStatus status{ GameStatus::WaitingForPlayers };
  };

  struct BoardView
  {
    std::array<std::array<battleship::CellState, battleship::BOARD_SIZE>, battleship::BOARD_SIZE> cells{};
  };

  struct GameView
  {
    GameStatus status{ GameStatus::WaitingForPlayers };
    int turn{ 0 };
    bool ready[2]{ false, false };
    BoardView boards[2];
  };

  struct AuthContext
  {
    int playerIndex{ -1 };  // 0/1, -1 => unauthorized
    std::string token;
  };

}  // namespace server
