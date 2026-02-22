#ifndef BOAT_HPP
#define BOAT_HPP

#include <cstdint>
#include <memory>

#include "structure.hpp"

namespace battleship
{
  enum class BoatType : std::uint8_t
  {
    CARRIER,    // 5 cells
    BATTLESHIP, // 4 cells
    CRUISER,    // 3 cells
    SUBMARINE,  // 3 cells
    DESTROYER   // 2 cells
  };

  class Boat : public Structure
  {
  public:
    explicit Boat(BoatType type);
    [[nodiscard]] BoatType getType() const noexcept;
    [[nodiscard]] StructureType type() const noexcept override;
    [[nodiscard]] std::unique_ptr<Structure> clone() const override;
    void reset() noexcept override;

  private:
    BoatType m_type;
    static constexpr std::uint8_t livesFor(BoatType type) noexcept;
  };
}

#endif //BOAT_HPP
