#include "project/core/board.hpp"

#include <gtest/gtest.h>

#include "project/core/boat.hpp"
#include "project/exceptions/exceptions.hpp"

namespace battleship::tests
{
  using battleship::Board;
  using battleship::Boat;
  using battleship::BoatType;
  using battleship::Cell;
  using battleship::CellState;
  using battleship::Coordinate;
  using battleship::Orientation;
  using battleship::Placement;

  class BoardFixture : public ::testing::Test
  {
   protected:
    Board board{};

    static Coordinate C(int row, int col)
    {
      return { row, col };
    }

    CellState Get(Coordinate c) const
    {
      return board.getCellView(c).cell_state;
    }

    void Set(Coordinate c, CellState state)
    {
      board.setCellView(c, Cell(state));
    }

    static void ExpectAllCells(const Board& b, CellState expected)
    {
      for (int r{ 0 }; r < BOARD_SIZE; ++r)
      {
        for (int c{ 0 }; c < BOARD_SIZE; ++c)
        {
          EXPECT_EQ(b.getCellView(C(r, c)).cell_state, expected) << "Mismatch at (" << r << "," << c << ")";
        }
      }
    }

    void ExpectAllCells(CellState expected)
    {
      ExpectAllCells(board, expected);
    }

    void ExpectCells(std::initializer_list<std::pair<int, int>> cells, CellState expected)
    {
      for (auto& [r, c] : cells)
      {
        EXPECT_EQ(Get(C(r, c)), expected) << "Mismatch at (" << r << "," << c << ")";
      }
    }

    void Place(BoatType boat, Coordinate c, Orientation o)
    {
      board.placeStructure(Boat{ boat }, Placement{ c, o });
    }

    void Shot(Coordinate c)
    {
      board.handle_shot(c);
    }
  };

  TEST_F(BoardFixture, NewBoard_AllCellsEmpty)
  {
    // GIVEN: a newly created board
    // WHEN:  no actions are performed
    // THEN:  all cells are empty
    ExpectAllCells(CellState::EMPTY);
  }

  TEST_F(BoardFixture, SetAndGetCellState_Works)
  {
    // GIVEN
    const Coordinate target{ 3, 4 };

    // WHEN
    Set(target, CellState::MISS);

    // THEN
    EXPECT_EQ(Get(target), CellState::MISS) << "Set cell didn't update the cell state correctly.";
  }

  TEST_F(BoardFixture, PlaceBoat_BoatCellsOccupied)
  {
    // GIVEN
    const Coordinate start{ 2, 2 };
    const Orientation orientation{ Orientation::EAST };

    // WHEN
    Place(BoatType::CRUISER, start, orientation);

    // THEN
    ExpectCells({ { 2, 2 }, { 2, 3 }, { 2, 4 } }, CellState::OCCUPIED);
  }

  TEST_F(BoardFixture, PlaceBoat_OutOfBounds_Throws)
  {
    // GIVEN
    const Coordinate start{ 9, 8 };
    const Orientation orientation{ Orientation::EAST };

    // WHEN & THEN
    EXPECT_THROW(Place(BoatType::CRUISER, start, orientation), OutOfBounds);
  }

  TEST_F(BoardFixture, PlaceBoat_Collision_Throws)
  {
    // GIVEN
    Place(BoatType::CRUISER, C(1, 1), Orientation::EAST);
    const Coordinate start{ 1, 0 };
    const Orientation orientation{ Orientation::SOUTH };

    // WHEN & THEN
    EXPECT_THROW(Place(BoatType::DESTROYER, start, orientation), Collision);
  }

  TEST_F(BoardFixture, ShotMiss_UpdatesCellState)
  {
    // GIVEN
    const Coordinate target{ 4, 4 };

    // WHEN
    Shot(target);

    // THEN
    EXPECT_EQ(Get(target), CellState::MISS) << "Shot in empty cell should update state to MISS.";
  }

  TEST_F(BoardFixture, ShotHit_UpdatesCellState)
  {
    // GIVEN
    Place(BoatType::DESTROYER, C(5, 5), Orientation::EAST);
    const Coordinate target{ 5, 5 };

    // WHEN
    Shot(target);

    // THEN
    EXPECT_EQ(Get(target), CellState::HIT) << "Shot in occupied cell should update state to HIT.";
  }

  TEST_F(BoardFixture, ShotAlreadyShot_Throws)
  {
    // GIVEN
    const Coordinate target{ 6, 6 };
    Shot(target);  // First shot to mark it as MISS

    // WHEN & THEN
    EXPECT_THROW(Shot(target), AlreadyShot);
  }

  TEST_F(BoardFixture, ResetBoard_CellsEmptyAndStructuresReset)
  {
    // GIVEN
    Place(BoatType::CRUISER, C(2, 2), Orientation::NORTH);
    Place(BoatType::DESTROYER, C(4, 4), Orientation::WEST);
    Shot(C(2, 2));  // Hit on cruiser
    Shot(C(4, 4));  // Hit on destroyer
    Shot(C(7, 7));  // Miss

    // WHEN
    board.reset();

    // THEN
    ExpectCells({ { 2, 2 }, { 1, 2 }, { 0, 2 } }, CellState::OCCUPIED);  // Cruiser cells should be occupied again
    ExpectCells({ { 4, 4 }, { 4, 3 } }, CellState::OCCUPIED);            // Destroyer cells should be occupied again
    ExpectCells({ { 7, 7 } }, CellState::EMPTY);                         // Missed cell should be empty again
  }

  TEST_F(BoardFixture, ShotOutOfBounds_Throws)
  {
    // GIVEN
    const Coordinate target{ 10, 10 };

    // WHEN & THEN
    EXPECT_THROW(Shot(target), OutOfBounds);
  }

  TEST_F(BoardFixture, OccupiedCellWithoutStructure_Throws)
  {
    // GIVEN
    Set(C(3, 3), CellState::OCCUPIED);  // Manually set cell to OCCUPIED without placing a structure

    // WHEN & THEN
    EXPECT_THROW(Shot(C(3, 3)), UndefinedShorError);
  }

  TEST_F(BoardFixture, BadOrientation_Throws)
  {
    // GIVEN
    const Coordinate start{ 1, 1 };
    const Orientation bad_orientation{ static_cast<Orientation>(999) };

    // WHEN & THEN
    EXPECT_THROW(Place(BoatType::CRUISER, start, bad_orientation), std::invalid_argument);
  }

}  // namespace battleship::tests