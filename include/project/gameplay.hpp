#ifndef PROJECT_GAMEPLAY_HPP
#define PROJECT_GAMEPLAY_HPP

#include <array>
#include <cstddef>

#include "project/player/player.hpp"

namespace battleship
{
  class GamePlay
  {
  public:
    GamePlay();

    GamePlay(const GamePlay&) = delete;
    GamePlay(GamePlay&&) = delete;
    GamePlay& operator=(const GamePlay&) = delete;
    GamePlay& operator=(GamePlay&&) = delete;
    ~GamePlay() = default;

    [[nodiscard]] Player& player(std::size_t index);
    [[nodiscard]] const Player& player(std::size_t index) const;

    [[nodiscard]] Player& playerById(int player_id);
    [[nodiscard]] const Player& playerById(int player_id) const;

    [[nodiscard]] Player& playerOne() noexcept;
    [[nodiscard]] const Player& playerOne() const noexcept;
    [[nodiscard]] Player& playerTwo() noexcept;
    [[nodiscard]] const Player& playerTwo() const noexcept;

    [[nodiscard]] Player& currentPlayer() noexcept;
    [[nodiscard]] const Player& currentPlayer() const noexcept;
    [[nodiscard]] Player& opponentPlayer() noexcept;
    [[nodiscard]] const Player& opponentPlayer() const noexcept;

    [[nodiscard]] int currentPlayerId() const noexcept;
    [[nodiscard]] int opponentPlayerId() const noexcept;
    [[nodiscard]] bool isPlayerTurn(int player_id) const noexcept;

    void switchTurn() noexcept;

    void placeBoat(int player_id, BoatType type, const Placement& placement);
    void placeBoatForCurrentPlayer(BoatType type, const Placement& placement);

    [[nodiscard]] CellState shoot(const Coordinate& target);
    [[nodiscard]] CellState shoot(int attacker_id, const Coordinate& target);

    [[nodiscard]] bool isGameOver() const noexcept;
    [[nodiscard]] bool hasWinner() const noexcept;
    [[nodiscard]] int winnerId() const noexcept;

    void reset();

  private:
    [[nodiscard]] std::size_t indexForPlayerId(int player_id) const;
    void updateWinnerAfterShot(std::size_t attacker_index, std::size_t defender_index) noexcept;

    std::array<Player, 2> m_players{ Player{ 1 }, Player{ 2 } };
    std::size_t m_turn_index{ 0 };
    int m_winner_id{ 0 };
  };

}  // namespace battleship

#endif  // PROJECT_GAMEPLAY_HPP
