#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <bitset>
#include <algorithm>
#include "Assembler.h"
#include "Parser.h"
#include "Coder.h"
#include "CommandType.h"

Assembler::Assembler(std::ifstream& inputFile, std::ofstream& outputFile)
  : m_inputFile(inputFile)
  , m_outputFile(outputFile)
{
}

std::string Assembler::handleAcommand(const std::string& symbol)
{
  int numberSymbol{};
  bool isNumber = std::all_of(symbol.begin(), symbol.end(), ::isdigit);
  bool isValidName = std::all_of(symbol.begin(), symbol.end(), [](char c) {
                      return std::isdigit(c) || std::isalpha(c) || c == '.' || c == ':' || c == '$' || c == '_';
                    });
  if ((!isNumber && isdigit(symbol[0])) || !isValidName) 
  {
    throw std::runtime_error("Invalid symbol in A_COMMAND: " + symbol);
  }
  
  if (!isNumber)
  {
    if (m_symbolTable.find(symbol) != m_symbolTable.end())
    {
      numberSymbol = m_symbolTable[symbol];
    }
    else
    {
      m_symbolTable[symbol] = m_nextVariableAddress;
      numberSymbol = m_symbolTable[symbol];
      m_nextVariableAddress++;
    }
  }
  else
  {
    try 
    {
      numberSymbol = std::stoi(symbol);
      if (numberSymbol < 0)
      {
        throw std::runtime_error("Negative number cannot be represented in A_COMMAND.");
      }
    }
    catch (const std::exception& error)
    {
      throw std::runtime_error("Invalid A_COMMAND format: @" + symbol);
    }
  }

  std::bitset<15> binaryNum(static_cast<unsigned int>(numberSymbol));

  return "0" + binaryNum.to_string();
}

void Assembler::generate()
{
  Parser parser(m_inputFile);
  Coder coder;

  int addressCounter{ 0 };
  while (parser.hasMoreCommands())
  {
    parser.advance();

    if(parser.commandType() == CommandType::L_COMMAND)
    {
      std::string symbol{ parser.symbol() };
      if (m_symbolTable.find(symbol) == m_symbolTable.end())
      {
        m_symbolTable[symbol] = addressCounter;
      }
      else
      {
        throw std::runtime_error("Invalid L_COMMAND. Duplicate symbol found: " + symbol);
      }
    }
    else
    {
      addressCounter++;
    }
  }
  parser.reset();
  
  while (parser.hasMoreCommands())
  {
    parser.advance();

    std::string line{};
    CommandType commandType{ parser.commandType() };

    bool lCommand{ false };

    switch (commandType)
    {
    case CommandType::L_COMMAND:
    {
      lCommand = true;
      break;
    }
    case CommandType::A_COMMAND:
    {
      line = handleAcommand(parser.symbol());
      break;
    }
    case CommandType::C_COMMAND:
    {
      std::string comp{ parser.comp() };
      std::string dest{ parser.dest() };
      std::string jump{ parser.jump() };
      line = "111" + coder.comp(comp) + coder.dest(dest) + coder.jump(jump);

      break;
    }
    default:
      throw std::runtime_error("Unknown command type encountered in Assembler::generate()");
    }

    if (lCommand) continue;

    if (line.size() != 16) 
    {
      throw std::runtime_error("Generated binary line does not have 16 bits: " + line);
    }

    line += '\n';
      
    m_outputFile << line;
  }
}