#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include <cstdint>

namespace battleship
{
  enum class StructureType : std::uint8_t
  {
    BOAT,
    MINE
  };

  class Structure
  {
  public:
    virtual ~Structure() = default;

    virtual void hit()
    {
      if (m_hp > 0)
      {
        --m_hp;
      }
    }

    [[nodiscard]] bool isDestroyed() const
    {
      return m_hp <= 0;
    }

    [[nodiscard]] std::uint8_t size() const
    {
      return m_size;
    }

    [[nodiscard]] virtual StructureType type() const noexcept = 0;
    [[nodiscard]]virtual std::unique_ptr<Structure> clone() const = 0;
    virtual void reset() noexcept = 0;

  protected:
    std::uint8_t m_hp;
    std::uint8_t m_size;
  };
}

#endif //STRUCTURES_HPP