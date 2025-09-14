#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>

namespace KeyWords
{
  enum class KeyWord
  {
    CLASS,
    METHOD,
    FUNCTION,
    CONSTRUCTOR,
    INT,
    BOOLEAN,
    CHAR,
    VOID,
    VAR,
    STATIC,
    FIELD,
    LET,
    DO,
    IF,
    ELSE,
    WHILE,
    RETURN,
    TRUE,
    FALSE,
    NULL_,
    THIS
  };

  // Function to convert KeyWord enum to string (in lowercase)
  std::string to_string(KeyWord keyword);

  // Function to map strings to KeyWords
  KeyWord fromString(const std::string& token);

  // Function to check if a token is a keyword
  bool isKeyword(const std::string& token);
}

#endif
