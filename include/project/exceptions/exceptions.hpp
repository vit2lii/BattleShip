#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <stdexcept>

namespace battleship
{
  struct Error : std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };

  struct PlacementError : Error
  {
    using Error::Error;
  };
  struct ShotError : Error
  {
    using Error::Error;
  };

  struct Collision : PlacementError
  {
    using PlacementError::PlacementError;
  };
  struct OutOfBounds : PlacementError
  {
    using PlacementError::PlacementError;
  };
  struct AlreadyShot : ShotError
  {
    using ShotError::ShotError;
  };

  struct UndefinedShorError : ShotError
  {
    using ShotError::ShotError;
  };

}  // namespace battleship

#endif  // EXCEPTIONS_HPP