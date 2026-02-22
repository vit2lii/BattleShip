#include <iostream>

#include "project/core/board.hpp"
#include "project/core/boat.hpp"
#include "project/core/coordinate.hpp"
#include "project/core/boardPrinter.hpp"

using namespace battleship;

int main()
{
  Board board{};

  Boat boat{ BoatType::CARRIER };
  Coordinate coordinate = Coordinate(1,1);
  Orientation orientation{ static_cast<Orientation>(999) };
  // board.placeStructure(boat, { coordinate, orientation });

  // Boat boat2 {BoatType::BATTLESHIP};
  // board.placeStructure(boat2, {Coordinate::parseFromString("F3"), Orientation::NORTH});
  printBoard(std::cout, board, {});

  return 0;
}