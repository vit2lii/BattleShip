#ifndef BOARD_HPP
#define BOARD_HPP

/**
 * @file board.hpp
 * @brief Battleship board configuration and types.
 *
 * This file defines the board size, cell states, and utility
 * functions for a standard Battleship game.
 */

#include <array>
#include <vector>
#include <cstdint>
#include <utility>
#include <memory>

#include "boat.hpp"
#include "coordinate.hpp"
#include "placement.hpp"
#include "cell.hpp"

namespace battleship
{
  constexpr std::uint8_t BOARD_SIZE = 10;
  constexpr std::uint8_t MAX_BOATS = 10;
  constexpr std::uint8_t MAX_MINES = 5;
  constexpr std::uint8_t MAX_STRUCTURES = MAX_BOATS + MAX_MINES;

  constexpr auto make_empty_cels()
  {
    std::array<std::array<Cell, BOARD_SIZE>, BOARD_SIZE> cells{};
    for (auto& row : cells)
    {
      row.fill(Cell{CellState::EMPTY});
    }

    return cells;
  }

  class Board
  {
  public:
    Board();
    ~Board() = default;

    // Non-copyable, non-movable
    Board(const Board&) = delete;
    Board& operator=(const Board&) = delete;
    Board(Board&&) = delete;

    Board& operator=(Board&&) = delete;

    [[nodiscard]] Cell getCellView(const Coordinate& coord) const;
    void setCellView(const Coordinate& coord, Cell cell_view);

    void placeStructure(const Structure& structure, const Placement& placement);
    void handle_shot(const Coordinate& coord);
    [[nodiscard]] bool allBoatsDestroyed() const;

    void reset();

  private:
    using StructureEntry = std::pair<std::unique_ptr<Structure>, Placement>;
    std::array<std::array<Cell, BOARD_SIZE>, BOARD_SIZE> m_cells = make_empty_cels();
    std::vector<StructureEntry> m_structuresStartPositions;

    void markStructureOnBoard(const Structure& structure, const Placement& placement) noexcept;
    void addStructureStartPosition(const Structure& structure, const Placement& placement);
    bool checkCollision(const Structure& structure, const Placement& placement);
    Structure& findStructureAt(const Coordinate& coord);
    Cell& cellAt(int row, int col);
  };

}
#endif // BOARD_HPP
