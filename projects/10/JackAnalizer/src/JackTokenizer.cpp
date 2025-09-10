#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include "IOFiles.h"
#include "TokenType.h"
#include "KeyWords.h"
#include "JackTokenizer.h"

JackTokenizer::JackTokenizer(InputFile& inputFile)
  : m_fileName(inputFile.fileName + ".jack")
  , m_tokenizedFile(tokenizeFile(inputFile.file))
  , m_hasMoreTokens(!m_tokenizedFile.empty())
  , m_currentTokenIndex(-1)
{
}

std::vector<JackTokenizer::Token> JackTokenizer::tokenizeFile(std::ifstream& file)
{
  std::vector<JackTokenizer::Token> tokenizedFile;
  std::string line{};
  int lineIdx{ -1 };
  
  // Define symbols to split on
  const std::string symbols = "{}()[],.;+-*/&|<>=~";

  bool multipleLinesBlockComment{ false };

  while (std::getline(file, line)) 
  {
    lineIdx++;

    // -------------------- 1) Remove single-line comments (//) --------------------

    size_t commentPos = line.find("//");
    if (commentPos != std::string::npos) 
    {
      line = line.substr(0, commentPos);
    }

    // -------------------- 2) Remove block comments (/* ... */) --------------------

    // If already inside a multi-line block comment (across multiple lines)
    if (multipleLinesBlockComment)
    {
      // Look for the end of the block comment (*/)
      size_t blockCommentEnd = line.find("*/");
      if (blockCommentEnd != std::string::npos) 
      {
        line = line.substr(blockCommentEnd + 2); // Remove the block comment part
        multipleLinesBlockComment = false;       // End the block comment flag
      }
      else 
      {
        line = "";   // The entire line is part of a comment, so just skip it
        continue;    // Skip processing this line, as it's inside a comment
      }
    }

    // Look for the start of a block comment (/*) in the current line
    size_t blockCommentStart = line.find("/*");
    while (blockCommentStart != std::string::npos) 
    {
      // Search for the end of the block comment (*/)
      size_t blockCommentEnd = line.find("*/", blockCommentStart + 2);
      if (blockCommentEnd != std::string::npos) 
      {
        // Remove the block comment (everything between /* and */)
        line = line.substr(0, blockCommentStart) + line.substr(blockCommentEnd + 2);
      }
      else 
      {
        // If no closing */, mark as multi-line comment and skip processing the rest of the line
        line = line.substr(0, blockCommentStart); 
        multipleLinesBlockComment = true;
        break;
      }
      blockCommentStart = line.find("/*"); // Look for another block comment in the same line
    }

    // If still inside a multi-line block comment, skip the current line
    if (multipleLinesBlockComment) continue;

    // -------------------- 3) Tokenize the line --------------------

    size_t start{ 0 };
    size_t end{ 0 };
    while (start < line.size()) 
    {
      // ----- If the current character is a symbol, treat it as a token -----
      if (symbols.find(line[start]) != std::string::npos) 
      {
        JackTokenizer::Token token{
          std::string(1, line[start]),  // symbol as a token
          lineIdx                        // line index
        };
        tokenizedFile.push_back(token);
        start++;  // Move past the symbol
      }
      // ----- Skip over any whitespace -----
      else if (isspace(line[start])) 
      {
        start++;
      }
      // ----- Handle string constants -----
      else if (line[start] == '\"') 
      {
        end = line.find('\"', start + 1);

        // Handle cases where the string is not properly terminated
        if (end == std::string::npos) 
        {
          throw std::runtime_error("Error in file " + m_fileName + " at line " + std::to_string(lineIdx) + ": "
                                   "Unterminated string constant");
        }

        JackTokenizer::Token token{
          line.substr(start, end - start + 1), // token: string literal (including quotes)
          lineIdx                              // line index
        };
        tokenizedFile.push_back(token);

        start = end + 1; 
      }
      // ----- Otherwise, we have a word (identifier, keyword, number, etc.) -----
      else 
      {
        end = line.find_first_of(" \t\r\n" + symbols, start);  // Find space or symbol
        if (end == std::string::npos) 
        {
          end = line.size();
        }

        JackTokenizer::Token token{
          line.substr(start, end - start), // token
          lineIdx                          // line index
        };
        tokenizedFile.push_back(token);

        start = end;
      }
    }
  }

  return tokenizedFile;
}

bool JackTokenizer::hasMoreTokens()
{
  return m_hasMoreTokens;
}

void JackTokenizer::advance()
{
  if (!m_hasMoreTokens)
  {
    return;
  }

  m_currentTokenIndex ++;
  size_t index = static_cast<size_t>(m_currentTokenIndex);
  m_currentToken = m_tokenizedFile[index];

  if (index == m_tokenizedFile.size() - 1)
  {
    m_hasMoreTokens = false;
  }
}

TokenType JackTokenizer::tokenType()
{
  const std::string symbols = "{}()[],.;+-*/&|<>=~";

  std::string token{ m_currentToken.token };
  int lineIdx{ m_currentToken.lineIdx };
  
  // SYMBOL
  if (symbols.find(token) != std::string::npos)
  {
    return TokenType::SYMBOL;
  }

  // KEYWORD
  if (KeyWords::isKeyword(token))
  {
    return TokenType::KEYWORD;
  }

  // STRING_CONST
  if (token[0] == '\"')
  {
    return TokenType::STRING_CONST;
  }

  // INT_CONST
  if (std::all_of(token.begin(), token.end(), ::isdigit))
  {
    return TokenType::INT_CONST;
  }

  // IDENTIFIER
  if (!std::isdigit(token[0]) && 
      std::all_of(token.begin(), token.end(), [](char c) {
        return std::isdigit(c) || std::isalpha(c) || c == '_';}))
  {
    return TokenType::IDENTIFIER;
  }

  throw std::runtime_error("Error in file " + m_fileName + " at line " + std::to_string(lineIdx) + ": "
                           "Unknown or invalid token: " + token);
}

KeyWords::KeyWord JackTokenizer::keyWord()
{
  if (tokenType() != TokenType::KEYWORD)
  {
    throw std::logic_error("Logic error in JackTokenizer::keyWord(): "
                           "current token is not of type TokenType::KEYWORD");
  }

  return KeyWords::fromString(m_currentToken.token);
}

char JackTokenizer::symbol()
{
  if (tokenType() != TokenType::SYMBOL)
  {
    throw std::logic_error("Logic error in JackTokenizer::symbol(): "
                           "current token is not of type TokenType::SYMBOL");
  }

  return m_currentToken.token[0];
}

std::string JackTokenizer::identifier()
{
  if (tokenType() != TokenType::IDENTIFIER)
  {
    throw std::logic_error("Logic error in JackTokenizer::identifier(): "
                           "current token is not of type TokenType::IDENTIFIER");
  }

  return m_currentToken.token;
}

int JackTokenizer::intVal()
{
  if (tokenType() != TokenType::INT_CONST)
  {
    throw std::logic_error("Logic error in JackTokenizer::intVal(): "
                           "current token is not of type TokenType::INT_CONST");
  }

  return std::stoi(m_currentToken.token);
}

std::string JackTokenizer::stringVal()
{
  if (tokenType() != TokenType::STRING_CONST)
  {
    throw std::logic_error("Logic error in JackTokenizer::stringVal(): "
                           "current token is not of type TokenType::STRING_CONST");
  }
  std::string quotedToken = m_currentToken.token;
  return quotedToken.substr(1, quotedToken.size() - 2);
}