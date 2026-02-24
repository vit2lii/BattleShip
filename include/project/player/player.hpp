#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "project/core/board.hpp"
#include "project/core/boat.hpp"
#include "project/core/placement.hpp"

/**
 * @file player.hpp
 * @brief Player class for Battleship game.
 */
namespace battleship
{
  class Player
  {
  public:
    Player() = default;
    explicit Player(int id) noexcept;

    Player(const Player&) = delete;
    Player(Player&&) = delete;
    Player& operator=(const Player&) = delete;
    Player& operator=(Player&&) = delete;

    [[nodiscard]] int id() const noexcept;
    [[nodiscard]] int getId() const noexcept;

    [[nodiscard]] Board& board() noexcept;
    [[nodiscard]] const Board& board() const noexcept;
    [[nodiscard]] Board& getBoard() noexcept;
    [[nodiscard]] const Board& getBoard() const noexcept;

    void placeBoat(BoatType type, const Placement& placement);
    void placeStructure(const Structure& structure, const Placement& placement);

    void receiveShot(const Coordinate& coord);

    [[nodiscard]] bool hasLost() const;
    [[nodiscard]] bool allBoatsDestroyed() const;

    void resetBoard();

  private:
    Board m_board;
    int m_id{};
  };

}  // namespace battleship

#endif  // PLAYER_HPP
