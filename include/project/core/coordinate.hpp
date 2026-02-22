#ifndef COORDINATE_HPP
#define COORDINATE_HPP

/**
 * @file coordinate.hpp
 * @brief Coordinate structure for Battleship game.
 *
 * This file defines the Coordinate struct used to represent
 * positions on the Battleship board.
 */

#include <cstdint>
#include <stdexcept>
#include <string>

namespace battleship
{
  /**
   * @brief Orientation of boats on Battleship board.
   */
  enum class Orientation : std::uint8_t
  {
    NORTH,
    WEST,
    SOUTH,
    EAST
  };

  /**
   * @brief Coordinate on the Battleship board.
   */
  struct Coordinate final
  {
    int row{};
    int col{};

    constexpr Coordinate() = default;
    constexpr Coordinate(int r, int c) : row(r), col(c)
    {
    }

    static Coordinate parseFromString(std::string_view s)
    {
      // trim spaces (optional but nice for user input)
      while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
      {
        s.remove_prefix(1);
      }
      while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
      {
        s.remove_suffix(1);
      }

      if (s.size() < 2 || s.size() > 3)
      {
        throw std::invalid_argument("Invalid coordinate string");
      }

      char rch = s[0];
      rch = static_cast<char>(std::toupper(static_cast<unsigned char>(rch)));

      if (rch < 'A' || rch > 'J')
      {
        throw std::invalid_argument("Invalid row character");
      }

      const int row = rch - 'A';

      int col = -1;
      if (s.size() == 2)
      {
        const char c1 = s[1];
        if (c1 < '1' || c1 > '9')
        {
          throw std::invalid_argument("Invalid column character");
        }

        col = (c1 - '0') - 1;  // '1'->0 ... '9'->8
      }
      else  // size == 3
      {
        if (!(s[1] == '1' && s[2] == '0'))
        {
          throw std::invalid_argument("Invalid column character");
        }

        col = 9;
      }

      return Coordinate{ row, col };
    }
  };
}  // namespace battleship

#endif  // COORDINATE_HPP
