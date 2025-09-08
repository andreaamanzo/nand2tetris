#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <map>
#include <string>
#include <fstream>
#include "Parser.h"
#include "Coder.h"

class Assembler
{
private:
  std::ifstream& m_inputFile;
  std::ofstream& m_outputFile;

  std::map<std::string, int> m_symbolTable
  {
    {"R0", 0},
    {"R1", 1},
    {"R2", 2},
    {"R3", 3},
    {"R4", 4},
    {"R5", 5},
    {"R6", 6},
    {"R7", 7},
    {"R8", 8},
    {"R9", 9},
    {"R10", 10},
    {"R11", 11},
    {"R12", 12},
    {"R13", 13},
    {"R14", 14},
    {"R15", 15},
    {"SP", 0},
    {"LCL", 1},
    {"ARG", 2},
    {"THIS", 3},
    {"THAT", 4},
    {"SCREEN", 16384},
    {"KBD", 24576}
  };

  int m_nextVariableAddress{ 16 };

  std::string handleAcommand(const std::string& symbol);
  
public:
  Assembler(std::ifstream& inputFile, std::ofstream& outputFile);
  void generate();
};

#endif