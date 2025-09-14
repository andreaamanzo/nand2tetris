#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <string>

namespace Identifiers
{
  enum class Category
  {
    CLASS,
    SUBROUTINE,
    VAR
  };

  enum class VarKind
  {
    STATIC,
    FIELD,
    ARG,
    VAR,
    NONE
  };

  enum class SubKind
  {
    CONSTRUCTOR,
    FUNCTION,
    METHOD,
    NONE
  };

  struct Symbol
  {
    std::string name;
    std::string type;
    VarKind     kind;
    int        index;
  };

  std::string to_string(Category identifierCategory);
}



#endif