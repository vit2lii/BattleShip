#ifndef PROJECT_CORE_CELL_HPP
#define PROJECT_CORE_CELL_HPP

#include <cstdint>

namespace battleship
{
  /**
   * @brief Cell states on the Battleship board for view purposes.
   */
  enum class CellState : std::uint8_t
  {
    EMPTY,
    OCCUPIED,
    HIT,
    MISS
  };

  /**
   * @brief View representation of a cell on the Battleship board.
   */
  struct Cell
  {
    CellState cell_state;
  };
}

#endif // PROJECT_CORE_CELL_HPP