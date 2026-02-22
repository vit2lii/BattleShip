#include "project/core/board.hpp"

#include <assert.h>

#include "project/exceptions/exceptions.hpp"

namespace battleship
{
  namespace
  {
    void advance(Coordinate& c, Orientation o)
    {
      switch (o)
      {
        case Orientation::NORTH: --c.row; break;
        case Orientation::SOUTH: ++c.row; break;
        case Orientation::WEST: --c.col; break;
        case Orientation::EAST: ++c.col; break;
        default: throw std::invalid_argument{ "Invalid orientation." };
      }
    }

    bool inBounds(int r, int c) noexcept
    {
      return r >= 0 && r < static_cast<int>(BOARD_SIZE) && c >= 0 && c < static_cast<int>(BOARD_SIZE);
    }

    template<typename Fn>
    bool anyCell(const Structure& structure, const Placement& placement, Fn&& fn)
    {
      const auto size{ structure.size() };
      Coordinate c{ placement.coordinate };

      for (std::uint8_t i{ 0 }; i < size; ++i)
      {
        if (fn(c))
        {
          return true;
        }

        advance(c, placement.orientation);
      }

      return false;
    }

    bool isInsideBoard(const Structure& structure, const Placement& placement)
    {
      return !anyCell(
          structure, placement, [](const Coordinate& c) { return c.row >= BOARD_SIZE || c.col >= BOARD_SIZE; });
    }
  }  // namespace

  Board::Board()
  {
    m_structuresStartPositions.reserve(MAX_STRUCTURES);
  }

  Cell Board::getCellView(const Coordinate& coord) const
  {
    return m_cells.at(static_cast<std::size_t>(coord.row)).at(static_cast<std::size_t>(coord.col));
  }

  void Board::setCellView(const Coordinate& coord, Cell cell_view)
  {
    m_cells.at(static_cast<std::size_t>(coord.row)).at(static_cast<std::size_t>(coord.col)) = cell_view;
  }

  Cell& Board::cellAt(int row, int col)
  {
    if (!inBounds(row, col))
    {
      throw OutOfBounds{ "Coordinate is out of board bounds." };
    }

    return m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
  }

  bool Board::checkCollision(const Structure& structure, const Placement& placement)
  {
    auto touchesOccupied = [&](const Coordinate& c) -> bool
    {
      for (int dr = -1; dr <= 1; ++dr)
      {
        for (int dc = -1; dc <= 1; ++dc)
        {
          const int row = c.row + dr;
          const int col = c.col + dc;

          if (!inBounds(row, col))
          {
            continue;
          }

          if (m_cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)].cell_state != CellState::EMPTY)
          {
            return true;
          }
        }
      }
      return false;
    };

    return anyCell(structure, placement, touchesOccupied);
  }

  void Board::addStructureStartPosition(const Structure& structure, const Placement& placement)
  {
    m_structuresStartPositions.emplace_back(structure.clone(), placement);
  }

  void Board::placeStructure(const Structure& structure, const Placement& placement)
  {
    if (!isInsideBoard(structure, placement))
    {
      throw OutOfBounds{ "Structure placement is out of board bounds." };
    }

    if (checkCollision(structure, placement))
    {
      throw Collision{ "Structure placement collides with existing structures." };
    }

    markStructureOnBoard(structure, placement);
    addStructureStartPosition(structure, placement);
  }

  void Board::markStructureOnBoard(const Structure& structure, const Placement& placement) noexcept
  {
    anyCell(
        structure,
        placement,
        [&](const Coordinate& c)
        {
          m_cells[static_cast<std::size_t>(c.row)][static_cast<std::size_t>(c.col)].cell_state = CellState::OCCUPIED;
          return false;
        });
  }

  Structure& Board::findStructureAt(const Coordinate& coord)
  {
    for (auto& [structure, placement] : m_structuresStartPositions)
    {
      Coordinate c = placement.coordinate;

      for (std::uint8_t i = 0; i < structure->size(); ++i)
      {
        if (c.row == coord.row && c.col == coord.col)
        {
          return *structure;
        }

        advance(c, placement.orientation);
      }
    }

    throw UndefinedShorError{ "Shot in occupied cell does not correspond to any structure." };
  }

  void Board::handle_shot(const Coordinate& coord)
  {
    auto& hit_cell = cellAt(coord.row, coord.col).cell_state;

    if (hit_cell == CellState::HIT || hit_cell == CellState::MISS)
    {
      throw AlreadyShot{ "Cell has already been shot." };
    }

    if (hit_cell == CellState::EMPTY)
    {
      hit_cell = CellState::MISS;
    }

    if (hit_cell == CellState::OCCUPIED)
    {
      findStructureAt(coord).hit();
      hit_cell = CellState::HIT;
    }
  }

  void Board::reset()
  {
    m_cells = make_empty_cels();

    for (auto& [structure, placement] : m_structuresStartPositions)
    {
      structure->reset();
      markStructureOnBoard(*structure, placement);
    }
  }

}  // namespace battleship