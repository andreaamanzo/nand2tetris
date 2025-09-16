#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
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

std::vector<std::string> Parser::getCurrentCommandTokens()
{
  std::vector<std::string> tokens{};
  size_t start{ 0 };
  size_t   end{ 0 };
  while ((end = m_currentCommand.find(' ', start)) != std::string::npos) 
  {
    tokens.push_back(m_currentCommand.substr(start, end - start));
    while (m_currentCommand[end] == ' ')
    {
      end++;
    }
    start = end;
  }
  tokens.push_back(m_currentCommand.substr(start));

  return tokens;
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
  std::string command{};
  size_t firstSpace = m_currentCommand.find(' ');
  if (firstSpace == std::string::npos) 
  {
    command = m_currentCommand;
  }
  command = m_currentCommand.substr(0, firstSpace);

  if (command == "push")
  {
    return CommandType::C_PUSH;
  }
  if (command == "pop")
  {
    return CommandType::C_POP;
  }
  if (command == "label")
  {
    return CommandType::C_LABEL;
  }
  if (command == "goto")
  {
    return CommandType::C_GOTO;
  }
  if (command == "if-goto")
  {
    return CommandType::C_IF;
  }
  if (command == "function")
  {
    return CommandType::C_FUNCTION;
  }
  if (command == "call")
  {
    return CommandType::C_CALL;
  }
  if (command == "return")
  {
    return CommandType::C_RETURN;
  }

  std::vector<std::string> arithmeticCommands 
  {
    "add",
    "sub",
    "neg",
    "eq",
    "gt",
    "lt",
    "and",
    "or",
    "not"
  };

  if (std::find(arithmeticCommands.begin(), arithmeticCommands.end(), command) != arithmeticCommands.end()) 
  {
    return CommandType::C_ARITHMETIC;
  }

  throw std::runtime_error("Unknown command: " + command);
}


std::string Parser::arg1()
{
  CommandType type{ commandType() };
  if (type == CommandType::C_RETURN)
  {
    throw std::logic_error("Parser::arg1() should not be called if the current command is C_RETURN");
  }
  // For arithmetic commands, return the command itself
  if (type == CommandType::C_ARITHMETIC)
  {
    return m_currentCommand;
  }
  // For other commands, return the first argument
  std::vector<std::string> tokens{ getCurrentCommandTokens() };

  if (tokens.size() >= 2) 
  {
    return tokens[1];
  }
  
  throw std::runtime_error("Command has insufficient arguments for arg1: " + m_currentCommand);
}

int Parser::arg2()
{
  CommandType type{ commandType() };
  if (type == CommandType::C_POP || type == CommandType::C_PUSH ||
      type == CommandType::C_FUNCTION || type == CommandType::C_CALL)
  {
    std::vector<std::string> tokens{ getCurrentCommandTokens() };
    if (tokens.size() >= 3)
    {
      try 
      {
        int value = std::stoi(tokens[2]);
        if (value < 0) 
        {
          throw std::runtime_error("Argument for arg2 must not be negative: " + m_currentCommand);
        }
        return value;
      } 
      catch (const std::invalid_argument& e) 
      {
        throw std::runtime_error("Invalid argument for arg2 (not an integer): " + m_currentCommand);
      } 
      catch (const std::out_of_range& e) 
      {
        throw std::runtime_error("Argument for arg2 is out of range: " + m_currentCommand);
      }
    }
    throw std::runtime_error("Command has insufficient arguments for arg2: " + m_currentCommand);
  }
  throw std::logic_error("Parser::arg2() should only be called for C_PUSH, C_POP, C_FUNCTION, or C_CALL commands");
}