#ifndef JACKTOKENIZER_H
#define JACKTOKENIZER_H

#include <vector>
#include <fstream>
#include <string>
#include "TokenType.h"
#include "KeyWords.h"
#include "IOFiles.h"

class JackTokenizer
{
private:
  struct Token 
  {
    std::string token;
    int lineIdx;
  };

  std::string m_fileName{};
  std::vector<Token> m_tokenizedFile{};
  bool m_hasMoreTokens{ false };
  int m_currentTokenIndex{ -1 };
  Token m_currentToken{};
  
  std::vector<Token> tokenizeFile(std::ifstream& file);

public:
  JackTokenizer(InputFile& inputFile);

  bool hasMoreTokens();
  void advance();
  int tokenLineIdx();
  TokenType tokenType();
  KeyWords::KeyWord keyWord();
  char symbol();
  std::string identifier();
  int intVal();
  std::string stringVal();
};

#endif