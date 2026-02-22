#ifndef BOARD_PRINTER_HPP
#define BOARD_PRINTER_HPP

#include <ostream>
#include <iomanip>
#include "project/core/board.hpp"

namespace battleship
{
  struct BoardPrintOptions
  {
    bool revealShips = true;     // show OCCUPIED as 'O' (your board). If false, hide as '.'
    bool showLegend  = true;
  };

  inline char cellToChar(CellState s, bool revealShips)
  {
    switch (s)
    {
      case CellState::EMPTY:    return '.';
      case CellState::OCCUPIED: return revealShips ? 'O' : '.';
      case CellState::HIT:      return 'X';
      case CellState::MISS:     return '*';
    }
    return '?'; // should never happen
  }

  inline void printBoard(std::ostream& os, const Board& board, BoardPrintOptions opt = {})
  {
    // Column header
    os << "    ";
    for (std::uint8_t col = 0; col < BOARD_SIZE; ++col)
    {
      os << std::setw(2) << col + 1 << ' ';
    }
    os << '\n';

    // Top border
    os << "   +";
    for (std::uint8_t col = 0; col < BOARD_SIZE; ++col) os << "---";
    os << "+\n";

    // Rows
    for (std::uint8_t row = 0; row < BOARD_SIZE; ++row)
    {
      const char rowLabel = static_cast<char>('A' + row);
      os << ' ' << rowLabel << " |";

      for (std::uint8_t col = 0; col < BOARD_SIZE; ++col)
      {
        Coordinate c{ row, col };
        const auto view = board.getCellView(c);
        os << ' ' << cellToChar(view.cell_state, opt.revealShips) << ' ';
      }

      os << "|\n";
    }

    // Bottom border
    os << "   +";
    for (std::uint8_t col = 0; col < BOARD_SIZE; ++col) os << "---";
    os << "+\n";

    if (opt.showLegend)
    {
      os << "Legend: . empty, O ship, X hit, * miss\n";
      if (!opt.revealShips) os << "(Ships hidden)\n";
    }
  }
} // namespace battleship


#endif //BOARD_PRINTER_HP