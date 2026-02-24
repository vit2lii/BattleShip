#include "project/gameplay.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace battleship::tests
{
  using battleship::BoatType;
  using battleship::CellState;
  using battleship::Coordinate;
  using battleship::GamePlay;
  using battleship::Orientation;
  using battleship::Placement;

  namespace
  {
    Placement placeAt(int row, int col, Orientation orientation)
    {
      return Placement{ Coordinate{ row, col }, orientation };
    }
  }

  TEST(GamePlayTest, NewGameStartsWithPlayerOneTurnAndNoWinner)
  {
    GamePlay game{};

    EXPECT_EQ(game.currentPlayerId(), 1);
    EXPECT_EQ(game.opponentPlayerId(), 2);
    EXPECT_FALSE(game.isGameOver());
    EXPECT_EQ(game.winnerId(), 0);
  }

  TEST(GamePlayTest, PlayerCanPlaceBoatById)
  {
    GamePlay game{};

    game.placeBoat(1, BoatType::DESTROYER, placeAt(0, 0, Orientation::EAST));

    EXPECT_EQ(game.playerById(1).board().getCellView(Coordinate{ 0, 0 }).cell_state, CellState::OCCUPIED);
    EXPECT_EQ(game.playerById(1).board().getCellView(Coordinate{ 0, 1 }).cell_state, CellState::OCCUPIED);
  }

  TEST(GamePlayTest, ShotMissSwitchesTurn)
  {
    GamePlay game{};

    game.placeBoat(1, BoatType::DESTROYER, placeAt(9, 9, Orientation::WEST));
    game.placeBoat(2, BoatType::DESTROYER, placeAt(9, 9, Orientation::WEST));

    const auto result = game.shoot(1, Coordinate{ 0, 0 });

    EXPECT_EQ(result, CellState::MISS);
    EXPECT_TRUE(game.isPlayerTurn(2));
    EXPECT_FALSE(game.isGameOver());
  }

  TEST(GamePlayTest, SinkingEnemyFleetSetsWinner)
  {
    GamePlay game{};

    game.placeBoat(1, BoatType::DESTROYER, placeAt(9, 9, Orientation::WEST));
    game.placeBoat(2, BoatType::DESTROYER, placeAt(0, 0, Orientation::EAST));

    EXPECT_EQ(game.shoot(1, Coordinate{ 0, 0 }), CellState::HIT);
    EXPECT_EQ(game.shoot(2, Coordinate{ 5, 5 }), CellState::MISS);
    EXPECT_EQ(game.shoot(1, Coordinate{ 0, 1 }), CellState::HIT);

    EXPECT_TRUE(game.isGameOver());
    EXPECT_EQ(game.winnerId(), 1);
    EXPECT_TRUE(game.hasWinner());
    EXPECT_EQ(game.currentPlayerId(), 1);
  }

  TEST(GamePlayTest, ShootingOutOfTurnThrows)
  {
    GamePlay game{};

    game.placeBoat(1, BoatType::DESTROYER, placeAt(9, 9, Orientation::WEST));
    game.placeBoat(2, BoatType::DESTROYER, placeAt(9, 9, Orientation::WEST));

    EXPECT_THROW((void)game.shoot(2, Coordinate{ 0, 0 }), std::logic_error);
  }

  TEST(GamePlayTest, UnknownPlayerIdThrows)
  {
    GamePlay game{};

    EXPECT_THROW((void)game.placeBoat(3, BoatType::DESTROYER, placeAt(0, 0, Orientation::EAST)),
                 std::invalid_argument);
  }

}  // namespace battleship::tests
