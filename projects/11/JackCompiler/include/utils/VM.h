#ifndef VM_H
#define VM_H

#include <string>

namespace VM
{
  enum class Segment
  {
    CONST,
    ARG,
    LOCAL,
    STATIC,
    THIS,
    THAT,
    POINTER,
    TEMP
  };

  enum class Command
  {
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT
  };

  std::string to_string(Segment segment);
  std::string to_string(Command command);

  Command command_from_char(char op, bool isUnary = false);
}

#endif