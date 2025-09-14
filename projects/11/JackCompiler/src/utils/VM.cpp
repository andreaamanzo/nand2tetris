#include <stdexcept>
#include "utils/VM.h"

std::string VM::to_string(Segment segment)
{
  switch (segment)
  {
  case Segment::CONST:   return "constant";
  case Segment::ARG:     return "argument";
  case Segment::LOCAL:   return "local";
  case Segment::STATIC:  return "static";
  case Segment::THIS:    return "this";
  case Segment::THAT:    return "that";
  case Segment::POINTER: return "pointer";
  case Segment::TEMP:    return "temp";
  default:               return "";
  }
}

std::string VM::to_string(Command command)
{
  switch (command)
  {
  case Command::ADD: return "add";
  case Command::SUB: return "sub";
  case Command::NEG: return "neg";
  case Command::EQ:  return "eq";
  case Command::GT:  return "gt";
  case Command::LT:  return "lt";
  case Command::AND: return "and";
  case Command::OR:  return "or";
  case Command::NOT: return "not";
  default:           return "";
  }
}

VM::Command VM::command_from_char(char op, bool isUnary /* =false */)
{
  switch (op)
  {
  case '+': return VM::Command::ADD;
  case '-': return (isUnary ? VM::Command::NEG : VM::Command::SUB);
  case '=': return VM::Command::EQ;
  case '<': return VM::Command::LT;
  case '>': return VM::Command::GT;
  case '&': return VM::Command::AND;
  case '|': return VM::Command::OR;
  case '~': return VM::Command::NOT;
  case '*': throw std::invalid_argument("Use Math.multiply for '*'");
  case '/': throw std::invalid_argument("Use Math.divide for '/'");
  default:
      throw std::invalid_argument(std::string("Unsupported operator: ") + op);
  }
}

