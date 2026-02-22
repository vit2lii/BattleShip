#include "project/core/boat.hpp"

#include <memory>

namespace battleship
{
  Boat::Boat(BoatType type) : m_type(type)
  {
    m_hp = livesFor(type);
    m_size = m_hp;
  }

  constexpr std::uint8_t Boat::livesFor(BoatType type) noexcept
  {
    switch (type)
    {
      case BoatType::CARRIER: return 5;
      case BoatType::BATTLESHIP: return 4;
      case BoatType::CRUISER: return 3;
      case BoatType::SUBMARINE: return 3;
      case BoatType::DESTROYER: return 2;
    }

    return 0;
  }

  BoatType Boat::getType() const noexcept
  {
    return m_type;
  }
  StructureType Boat::type() const noexcept
  {
    return StructureType::BOAT;
  }

  void Boat::reset() noexcept
  {
    m_hp = m_size;
  }

  std::unique_ptr<Structure> Boat::clone() const
  {
    return std::make_unique<Boat>(*this);
  }

}  // namespace battleship