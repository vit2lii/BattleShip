#include <array>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "project/core/boardPrinter.hpp"
#include "project/gameplay.hpp"

namespace battleship
{
  namespace
  {
    struct FleetBoat
    {
      BoatType type;
      const char* name;
      int size;
    };

    constexpr std::array<FleetBoat, 5> DEFAULT_FLEET{
      FleetBoat{ .type=BoatType::CARRIER, .name="Carrier", .size=5 },
      FleetBoat{ .type=BoatType::BATTLESHIP, .name="Battleship", .size=4 },
      FleetBoat{ .type=BoatType::CRUISER, .name="Cruiser", .size=3 },
      FleetBoat{ .type=BoatType::SUBMARINE, .name="Submarine", .size=3 },
      FleetBoat{ .type=BoatType::DESTROYER, .name="Destroyer", .size=2 },
    };

    void printSeparator()
    {
      std::cout << "\n----------------------------------------\n";
    }

    Orientation parseOrientation(const std::string& token)
    {
      if (token.size() != 1)
      {
        throw std::invalid_argument("Invalid orientation. Use N/S/E/W.");
      }

      switch (std::toupper(static_cast<unsigned char>(token[0])))
      {
        case 'N': return Orientation::NORTH;
        case 'S': return Orientation::SOUTH;
        case 'E': return Orientation::EAST;
        case 'W': return Orientation::WEST;
        default: throw std::invalid_argument("Invalid orientation. Use N/S/E/W.");
      }
    }

    std::pair<Coordinate, Orientation> readPlacementInput()
    {
      std::string line;
      std::getline(std::cin, line);

      std::istringstream iss{ line };
      std::string coord_token;
      std::string orientation_token;
      if (!(iss >> coord_token >> orientation_token))
      {
        throw std::invalid_argument("Input format: <COORDINATE> <ORIENTATION> (example: A1 E).");
      }

      return { Coordinate::parseFromString(coord_token), parseOrientation(orientation_token) };
    }

    Coordinate readShotInput()
    {
      std::string line;
      std::getline(std::cin, line);
      return Coordinate::parseFromString(line);
    }

    void placeFleet(GamePlay& game, int player_id)
    {
      printSeparator();
      std::cout << "Player " << player_id << ", place your fleet.\n";
      std::cout << "Format: <COORDINATE> <ORIENTATION>, e.g. A1 E\n";
      std::cout << "Orientation: N S E W\n\n";

      for (const auto& boat : DEFAULT_FLEET)
      {
        bool placed = false;
        while (!placed)
        {
          std::cout << boat.name << " (" << boat.size << "): ";
          try
          {
            auto [coordinate, orientation] = readPlacementInput();
            game.placeBoat(player_id, boat.type, Placement{ coordinate, orientation });
            placed = true;
            printBoard(std::cout, game.playerById(player_id).board(), BoardPrintOptions{ .revealShips=true, .showLegend=false });
          }
          catch (const std::exception& e)
          {
            std::cout << "Placement failed: " << e.what() << '\n';
          }
        }
      }
    }

    void playTurns(GamePlay& game)
    {
      while (!game.isGameOver())
      {
        const int attacker_id = game.currentPlayerId();
        const int defender_id = game.opponentPlayerId();

        printSeparator();
        std::cout << "Player " << attacker_id << " turn.\n\n";
        std::cout << "Your board:\n";
        printBoard(std::cout, game.playerById(attacker_id).board(), BoardPrintOptions{ .revealShips=true, .showLegend=true });

        std::cout << "\nEnemy board:\n";
        printBoard(std::cout, game.playerById(defender_id).board(), BoardPrintOptions{ .revealShips=false, .showLegend=true });

        bool shot_done = false;
        while (!shot_done)
        {
          std::cout << "\nShoot at coordinate (example: B7): ";
          try
          {
            const Coordinate target = readShotInput();
            const CellState result = game.shoot(attacker_id, target);
            std::cout << ((result == CellState::HIT) ? "Hit!\n" : "Miss.\n");
            shot_done = true;
          }
          catch (const std::exception& e)
          {
            std::cout << "Shot failed: " << e.what() << '\n';
          }
        }
      }
    }
  }  // namespace
}  // namespace battleship

int main()
{
  using namespace battleship;

  GamePlay game{};

  std::cout << "Battleship Console (2 players)\n";
  std::cout << "Board size: " << static_cast<int>(BOARD_SIZE) << "x" << static_cast<int>(BOARD_SIZE) << '\n';

  placeFleet(game, 1);
  placeFleet(game, 2);
  playTurns(game);

  printSeparator();
  std::cout << "Game over. Winner: Player " << game.winnerId() << '\n';
  std::cout << "Final board of Player 1:\n";
  printBoard(std::cout, game.playerById(1).board(), BoardPrintOptions{ .revealShips=true, .showLegend=true });
  std::cout << "\nFinal board of Player 2:\n";
  printBoard(std::cout, game.playerById(2).board(), BoardPrintOptions{ .revealShips=true, .showLegend=true });

  return 0;
}
