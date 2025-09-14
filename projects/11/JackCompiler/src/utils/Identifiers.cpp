#include <string>
#include "utils/Identifiers.h"

std::string Identifiers::to_string(Category identifierCategory)
{
  switch (identifierCategory)
  {
    case Category::CLASS:      return "class";
    case Category::SUBROUTINE: return "subroutine";
    case Category::VAR:        return "var";
    default:                         return ""; // Should not reach here
  }
}