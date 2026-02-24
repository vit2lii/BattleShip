#include "project/player/player.hpp"

namespace battleship
{
  Player::Player(int id) noexcept : m_id(id)
  {
  }

  int Player::id() const noexcept
  {
    return m_id;
  }

  int Player::getId() const noexcept
  {
    return id();
  }

  Board& Player::board() noexcept
  {
    return m_board;
  }

  const Board& Player::board() const noexcept
  {
    return m_board;
  }

  Board& Player::getBoard() noexcept
  {
    return board();
  }

  const Board& Player::getBoard() const noexcept
  {
    return board();
  }

  void Player::placeBoat(BoatType type, const Placement& placement)
  {
    placeStructure(Boat{ type }, placement);
  }

  void Player::placeStructure(const Structure& structure, const Placement& placement)
  {
    m_board.placeStructure(structure, placement);
  }

  void Player::receiveShot(const Coordinate& coord)
  {
    m_board.handle_shot(coord);
  }

  bool Player::hasLost() const
  {
    return allBoatsDestroyed();
  }

  bool Player::allBoatsDestroyed() const
  {
    return m_board.allBoatsDestroyed();
  }

  void Player::resetBoard()
  {
    m_board.reset();
  }

}  // namespace battleship
