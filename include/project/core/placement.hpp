#ifndef PLACEMENT_HPP
#define PLACEMENT_HPP
#include "coordinate.hpp"

/**
 * @file placement.hpp
 * @brief Placement structure for Battleship boats.
 */
namespace battleship
{
  /**
   * @brief Placement of a boat on the Battleship board.
   */
  struct Placement
  {
    Coordinate coordinate;
    Orientation orientation;

    constexpr Placement(Coordinate c, Orientation o) : coordinate(c), orientation(o) {}
  };
}

#endif //PLACEMENT_HPP
