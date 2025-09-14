#include <string>
#include <set>
#include <stdexcept>
#include "utils/KeyWords.h"

std::string KeyWords::to_string(KeyWord keyword)
{
  switch (keyword)
  {
    case KeyWord::CLASS:       return "class";
    case KeyWord::METHOD:      return "method";
    case KeyWord::FUNCTION:    return "function";
    case KeyWord::CONSTRUCTOR: return "constructor";
    case KeyWord::INT:         return "int";
    case KeyWord::BOOLEAN:     return "boolean";
    case KeyWord::CHAR:        return "char";
    case KeyWord::VOID:        return "void";
    case KeyWord::VAR:         return "var";
    case KeyWord::STATIC:      return "static";
    case KeyWord::FIELD:       return "field";
    case KeyWord::LET:         return "let";
    case KeyWord::DO:          return "do";
    case KeyWord::IF:          return "if";
    case KeyWord::ELSE:        return "else";
    case KeyWord::WHILE:       return "while";
    case KeyWord::RETURN:      return "return";
    case KeyWord::TRUE:        return "true";
    case KeyWord::FALSE:       return "false";
    case KeyWord::NULL_:       return "null";
    case KeyWord::THIS:        return "this";
    default:                   return ""; // Should not reach here
  }
}

  // Function to map strings to KeyWords
  KeyWords::KeyWord KeyWords::fromString(const std::string& token)
  {
    if (token == "class")       return KeyWord::CLASS;
    if (token == "method")      return KeyWord::METHOD;
    if (token == "function")    return KeyWord::FUNCTION;
    if (token == "constructor") return KeyWord::CONSTRUCTOR;
    if (token == "int")         return KeyWord::INT;
    if (token == "boolean")     return KeyWord::BOOLEAN;
    if (token == "char")        return KeyWord::CHAR;
    if (token == "void")        return KeyWord::VOID;
    if (token == "var")         return KeyWord::VAR;
    if (token == "static")      return KeyWord::STATIC;
    if (token == "field")       return KeyWord::FIELD;
    if (token == "let")         return KeyWord::LET;
    if (token == "do")          return KeyWord::DO;
    if (token == "if")          return KeyWord::IF;
    if (token == "else")        return KeyWord::ELSE;
    if (token == "while")       return KeyWord::WHILE;
    if (token == "return")      return KeyWord::RETURN;
    if (token == "true")        return KeyWord::TRUE;
    if (token == "false")       return KeyWord::FALSE;
    if (token == "null")        return KeyWord::NULL_;
    if (token == "this")        return KeyWord::THIS;

    // If the string doesn't match any known keyword, throw an error
    throw std::invalid_argument("KeyWords::fromString(): Invalid keyword: " + token);
  }

  bool KeyWords::isKeyword(const std::string& token)
  {
    // Create a set of all keyword strings
    static const std::set<std::string> keywords = {
      to_string(KeyWord::CLASS),
      to_string(KeyWord::METHOD),
      to_string(KeyWord::FUNCTION),
      to_string(KeyWord::CONSTRUCTOR),
      to_string(KeyWord::INT),
      to_string(KeyWord::BOOLEAN),
      to_string(KeyWord::CHAR),
      to_string(KeyWord::VOID),
      to_string(KeyWord::VAR),
      to_string(KeyWord::STATIC),
      to_string(KeyWord::FIELD),
      to_string(KeyWord::LET),
      to_string(KeyWord::DO),
      to_string(KeyWord::IF),
      to_string(KeyWord::ELSE),
      to_string(KeyWord::WHILE),
      to_string(KeyWord::RETURN),
      to_string(KeyWord::TRUE),
      to_string(KeyWord::FALSE),
      to_string(KeyWord::NULL_),
      to_string(KeyWord::THIS)
    };

    return keywords.find(token) != keywords.end();
  }
