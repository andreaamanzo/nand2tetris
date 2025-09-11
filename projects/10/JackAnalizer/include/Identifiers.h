#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <string>

namespace Identifiers
{
  enum class Identifier
  {
    CLASS,
    SUBROUTINE,
    VAR
  };

  // Function to convert Identifier enum to string (in lowercase)
  std::string to_string(Identifier keyword);
}



#endif