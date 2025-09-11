#include <string>
#include "Identifiers.h"

std::string Identifiers::to_string(Identifier identifier)
{
  switch (identifier)
  {
    case Identifier::CLASS:      return "class";
    case Identifier::SUBROUTINE: return "subroutine";
    case Identifier::VAR:        return "var";
    default:                     return ""; // Should not reach here
  }
}