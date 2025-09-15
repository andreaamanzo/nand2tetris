#ifndef JACKTOKENIZER_H
#define JACKTOKENIZER_H

#include <vector>
#include <fstream>
#include <string>
#include "utils/TokenType.h"
#include "utils/KeyWords.h"
#include "utils/InputFile.h"

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
  
  std::vector<Token> tokenizeFile(std::ifstream& file) const;

public:
  JackTokenizer(InputFile& inputFile);

  bool hasMoreTokens() const noexcept;
  void advance() noexcept;
  int tokenLineIdx() const noexcept;
  TokenType tokenType() const;
  KeyWords::KeyWord keyWord() const;
  char symbol() const;
  std::string identifier() const;
  int intVal() const;
  std::string stringVal() const;
};

#endif