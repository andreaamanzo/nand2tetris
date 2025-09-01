#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <fstream>
#include "CommandType.h"

class Parser 
{
private:
  std::vector<std::string> m_cleanedFile{};
  bool m_hasMoreCommands{ false };
  int m_currentCommandIndex{ -1 };
  std::string m_currentCommand{};

  std::vector<std::string> cleanFile(std::ifstream& file);

public:
  Parser(std::ifstream& inputFile);

  bool hasMoreCommands();
  void advance();
  CommandType commandType();
  std::string symbol();
  std::string dest();
  std::string comp();
  std::string jump();
  void reset();
};

#endif