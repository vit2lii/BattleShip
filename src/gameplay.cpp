#include "project/gameplay.hpp"

#include <stdexcept>

namespace battleship
{
  namespace
  {
    constexpr std::size_t PLAYER_COUNT = 2;
    constexpr std::size_t FIRST_PLAYER_INDEX = 0;
    constexpr std::size_t SECOND_PLAYER_INDEX = 1;
  }

  GamePlay::GamePlay() = default;

  Player& GamePlay::player(std::size_t index)
  {
    if (index >= m_players.size())
    {
      throw std::out_of_range{ "Player index is out of range." };
    }

    return m_players[index];
  }

  const Player& GamePlay::player(std::size_t index) const
  {
    if (index >= m_players.size())
    {
      throw std::out_of_range{ "Player index is out of range." };
    }

    return m_players[index];
  }

  Player& GamePlay::playerById(int player_id)
  {
    return player(indexForPlayerId(player_id));
  }

  const Player& GamePlay::playerById(int player_id) const
  {
    return player(indexForPlayerId(player_id));
  }

  Player& GamePlay::playerOne() noexcept
  {
    return m_players[FIRST_PLAYER_INDEX];
  }

  const Player& GamePlay::playerOne() const noexcept
  {
    return m_players[FIRST_PLAYER_INDEX];
  }

  Player& GamePlay::playerTwo() noexcept
  {
    return m_players[SECOND_PLAYER_INDEX];
  }

  const Player& GamePlay::playerTwo() const noexcept
  {
    return m_players[SECOND_PLAYER_INDEX];
  }

  Player& GamePlay::currentPlayer() noexcept
  {
    return m_players[m_turn_index];
  }

  const Player& GamePlay::currentPlayer() const noexcept
  {
    return m_players[m_turn_index];
  }

  Player& GamePlay::opponentPlayer() noexcept
  {
    return m_players[SECOND_PLAYER_INDEX - m_turn_index];
  }

  const Player& GamePlay::opponentPlayer() const noexcept
  {
    return m_players[SECOND_PLAYER_INDEX - m_turn_index];
  }

  int GamePlay::currentPlayerId() const noexcept
  {
    return currentPlayer().id();
  }

  int GamePlay::opponentPlayerId() const noexcept
  {
    return opponentPlayer().id();
  }

  bool GamePlay::isPlayerTurn(int player_id) const noexcept
  {
    return currentPlayerId() == player_id;
  }

  void GamePlay::switchTurn() noexcept
  {
    m_turn_index = SECOND_PLAYER_INDEX - m_turn_index;
  }

  void GamePlay::placeBoat(int player_id, BoatType type, const Placement& placement)
  {
    if (isGameOver())
    {
      throw std::logic_error{ "Game is over." };
    }

    playerById(player_id).placeBoat(type, placement);
  }

  void GamePlay::placeBoatForCurrentPlayer(BoatType type, const Placement& placement)
  {
    placeBoat(currentPlayerId(), type, placement);
  }

  CellState GamePlay::shoot(const Coordinate& target)
  {
    return shoot(currentPlayerId(), target);
  }

  CellState GamePlay::shoot(int attacker_id, const Coordinate& target)
  {
    if (isGameOver())
    {
      throw std::logic_error{ "Game is over." };
    }

    const std::size_t attacker_index = indexForPlayerId(attacker_id);
    if (attacker_index != m_turn_index)
    {
      throw std::logic_error{ "Not this player's turn." };
    }

    const std::size_t defender_index = SECOND_PLAYER_INDEX - attacker_index;
    Player& defender = m_players[defender_index];
    defender.receiveShot(target);

    const CellState shot_result = defender.board().getCellView(target).cell_state;
    updateWinnerAfterShot(attacker_index, defender_index);

    if (!isGameOver())
    {
      switchTurn();
    }

    return shot_result;
  }

  bool GamePlay::isGameOver() const noexcept
  {
    return m_winner_id != 0;
  }

  bool GamePlay::hasWinner() const noexcept
  {
    return isGameOver();
  }

  int GamePlay::winnerId() const noexcept
  {
    return m_winner_id;
  }

  void GamePlay::reset()
  {
    for (auto& p : m_players)
    {
      p.resetBoard();
    }

    m_turn_index = FIRST_PLAYER_INDEX;
    m_winner_id = 0;
  }

  std::size_t GamePlay::indexForPlayerId(int player_id) const
  {
    for (std::size_t i = 0; i < PLAYER_COUNT; ++i)
    {
      if (m_players[i].id() == player_id)
      {
        return i;
      }
    }

    throw std::invalid_argument{ "Unknown player id." };
  }

  void GamePlay::updateWinnerAfterShot(std::size_t attacker_index, std::size_t defender_index) noexcept
  {
    if (m_players[defender_index].hasLost())
    {
      m_winner_id = m_players[attacker_index].id();
    }
  }

}  // namespace battleship
