#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "vast-checker" ? 0 : 1;
}
