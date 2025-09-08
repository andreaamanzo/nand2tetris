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

  std::vector<std::string> cleanFile(std::ifstream& file);
  std::vector<std::string> getCurrentCommandTokens();
  
public:
  Parser(std::ifstream& inputFile);

  bool hasMoreCommands();
  void advance();
  CommandType commandType();
  std::string arg1();
  int arg2();
};

#endif