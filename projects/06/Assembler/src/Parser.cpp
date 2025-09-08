#include <string>
#include <fstream>
#include <iostream>
 #include <map>
#include "Parser.h"
#include "CommandType.h"

Parser::Parser(std::ifstream& inputFile)
  : m_cleanedFile(cleanFile(inputFile))
  , m_hasMoreCommands(!m_cleanedFile.empty())
  , m_currentCommandIndex(-1)
{
}

std::vector<std::string> Parser::cleanFile(std::ifstream& file)
{
  std::vector<std::string> cleanedFile;
  std::string line;
  while (std::getline(file, line)) {
    // Remove comments
    size_t commentPos = line.find("//");
    if (commentPos != std::string::npos) {
      line = line.substr(0, commentPos);
    }
    // Trim leading and trailing whitespace
    size_t first = line.find_first_not_of(" \t\r\n");
    size_t last = line.find_last_not_of(" \t\r\n");
    if (first != std::string::npos && last != std::string::npos) {
      line = line.substr(first, last - first + 1);
      if (!line.empty()) {
        cleanedFile.push_back(line);
      }
    }
  }
  return cleanedFile;
}

bool Parser::hasMoreCommands()
{
  return m_hasMoreCommands;
}

void Parser::advance()
{
  if (!m_hasMoreCommands)
  {
    return;
  }

  m_currentCommandIndex ++;
  size_t index = static_cast<size_t>(m_currentCommandIndex);
  m_currentCommand = m_cleanedFile[index];

  if (index == m_cleanedFile.size() - 1)
  {
    m_hasMoreCommands = false;
  }
}

CommandType Parser::commandType()
{
  char c{ m_currentCommand[0] };

  switch (c)
  {
  case '@':
    return CommandType::A_COMMAND;
  case '(':
    return CommandType::L_COMMAND;
  default:
    return CommandType::C_COMMAND;
  }
}

std::string Parser::symbol()
{
  CommandType type{ commandType() };
  if (type == CommandType::L_COMMAND) 
  {
    // Extract symbol between '(' and ')'
    size_t end{ m_currentCommand.find(')') };
    if ((end == std::string::npos) || (end != m_currentCommand.size() - 1) || (m_currentCommand.size() < 3)) 
    {
      throw std::runtime_error("Invalid L_COMMAND format: " + m_currentCommand);
    }
    return m_currentCommand.substr(1, end - 1);
  }
  else if (type == CommandType::A_COMMAND)
  {
    // Extract symbol after '@'
    if (m_currentCommand.size() < 2) 
    {
      throw std::runtime_error("Invalid A_COMMAND format " + m_currentCommand);
    }
    return m_currentCommand.substr(1);
  }
  else
  {
    throw std::logic_error("Parser::symbol can only be called if the current command is of type A_COMMAND or L_COMMAND");
  }
}

std::string Parser::dest()
{
  CommandType type{ commandType() };
  if (type != CommandType::C_COMMAND)
  {
    throw std::logic_error("Parser::dest can only be called if the current command is of type C_COMMAND");
  }

  size_t pos { m_currentCommand.find('=') };
  if (pos == std::string::npos)
  {
    return ""; // null
  }
  // Extract string before '='
  return m_currentCommand.substr(0, pos);
}

std::string Parser::comp()
{
  CommandType type{ commandType() };
  if (type != CommandType::C_COMMAND)
  {
    throw std::logic_error("Parser::comp can only be called if the current command is of type C_COMMAND");
  }

  size_t pos_dest { m_currentCommand.find('=') };
  size_t pos_jump { m_currentCommand.find(';') };
  // comp can't be omitted
  if ((pos_dest == std::string::npos) && (pos_jump == std::string::npos)) 
  {
    throw std::runtime_error("Invalid C_COMMAND format: " + m_currentCommand); 
  }
  // Format dest=comp
  if (pos_jump == std::string::npos) 
  {
    return m_currentCommand.substr(pos_dest + 1); 
  }
  // Format comp;jump
  return m_currentCommand.substr(0, pos_jump);
}

std::string Parser::jump()
{
  CommandType type{ commandType() };
  if (type != CommandType::C_COMMAND)
  {
    throw std::logic_error("Parser::jump can only be called if the current command is of type C_COMMAND");
  }

  size_t pos { m_currentCommand.find(';') };
  if (pos == std::string::npos)
  {
    return ""; // null
  }

  // Extract string after ';'
  return m_currentCommand.substr(pos + 1);
}

void Parser::reset()
{
  m_currentCommandIndex = -1;
  m_hasMoreCommands = !m_cleanedFile.empty();
  m_currentCommand.clear();
}