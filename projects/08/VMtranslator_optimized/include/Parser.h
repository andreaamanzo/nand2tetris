#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <fstream>
#include <string>
#include "CommandType.h"

class Parser
{
private:
  std::vector<std::string> m_cleanedFile{};
  bool m_hasMoreCommands{ false };
  int m_currentCommandIndex{ -1 };
  std::string m_currentCommand{};

  std::vector<std::string> cleanFile(std::ifstream& file) const;
  std::vector<std::string> getCurrentCommandTokens() const;
  
public:
  Parser(std::ifstream& inputFile);

  bool hasMoreCommands() const noexcept;
  void advance();
  CommandType commandType() const;
  std::string arg1() const;
  int arg2() const;
};

#endif