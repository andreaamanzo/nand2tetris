#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include "CommandType.h"
#include "CodeWriter.h"

CodeWriter::CodeWriter(std::ofstream& outputFile)
  : m_outputFile(outputFile)
{
}

std::string CodeWriter::uniqueLabel(const std::string& jmp) 
{
  int* ctr =
    (jmp == "EQ") ? &m_eqLabelId :
    (jmp == "GT") ? &m_gtLabelId :
    (jmp == "LT") ? &m_ltLabelId : nullptr;

  if (!ctr)
    throw std::invalid_argument("Unknown jump type: " + jmp);

  int id = (*ctr)++; 
  return jmp + "_END_" + std::to_string(id);
}

std::string CodeWriter::emitBinary(const std::string& op) 
{
  // pop y in D, punta a x e applica: x = x op y
  std::string last = (op == "-" ? ("M=M" + op + "D\n") : ("M=D" + op + "M\n"));
  
  return
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "A=A-1\n"
    + last;
}

std::string CodeWriter::emitUnary(const std::string& op) 
{
  // punta a top e applica: x = op x
  return
    "@SP\n"
    "A=M-1\n"
    "M=" + op + "M\n";
}

std::string CodeWriter::emitCompare(const std::string& jmp) 
{
  const std::string label = uniqueLabel(jmp);
  return
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "A=A-1\n"
    "D=M-D\n"          // D = x - y
    "@SP\n"
    "A=M-1\n"
    "M=-1\n"           // preset true (-1)
    "@" + label + "\n"
    "D;J" + jmp + "\n" // se condizione soddisfatta, salta: resta -1
    "@SP\n"
    "A=M-1\n"
    "M=0\n"            // altrimenti false (0)
    "(" + label + ")\n";
}

std::string CodeWriter::emitMemorySegment(const std::string& segment, int index)
{
  if (segment == "temp" && (index < 0 || index > 7))
    throw std::invalid_argument("temp index out of range");
  if (segment == "pointer" && (index < 0 || index > 1))
    throw std::invalid_argument("pointer index out of range");
  if (index < 0)
    throw std::invalid_argument("negative index");

  if (segment == "constant") 
  {
    return 
      "@" + std::to_string(index) + "\n";
  } 
  else if (segment == "temp" || segment == "pointer") 
  {
    std::string location =
      (segment == "pointer")  ? std::to_string(3 + index) : std::to_string(5 + index);

    return 
      "@" + location + "\n";
  } 
  else if (segment == "static") 
  {
    return
      "@" + m_fileName + "." + std::to_string(index) + "\n";
  }

  std::string base =
    (segment == "local")    ? "LCL" :
    (segment == "argument") ? "ARG" :
    (segment == "this")     ? "THIS" :
    (segment == "that")     ? "THAT" :
    "";
  
  if (base == "") 
    throw std::invalid_argument("Unknown memory segment: " + segment);

  return
    "@" + std::to_string(index) + "\n"
    "D=A\n"
    "@" + base + "\n"
    "A=D+M\n";
}

std::string CodeWriter::emitPush(const std::string& segment, int index)
{
  std::string code{ emitMemorySegment(segment, index) };
  if (segment == "constant") 
    code += "D=A\n";
  else
    code += "D=M\n";

  code +=
    "@SP\n"
    "AM=M+1\n"
    "A=A-1\n"
    "M=D\n";

  return code;
}

std::string CodeWriter::emitPop(const std::string& segment, int index)
{
  if (segment == "constant")
    throw std::invalid_argument("Cannot pop to constant");

  // Calcola l’indirizzo target e lo salva in R13
  std::string code{ emitMemorySegment(segment, index) };
  code +=
    "D=A\n"
    "@R13\n"
    "M=D\n"

    // Pop dello stack in D
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"

    // Scrive il pop (D) in *R13 
    "@R13\n"
    "A=M\n"
    "M=D\n";

  return code;
}

void CodeWriter::setFileName(const std::string& fileName)
{
  m_fileName = fileName;
}

void CodeWriter::writeArithmetic(const std::string& command)
{
  std::string code{};

  if      (command == "add") code = emitBinary("+");
  else if (command == "sub") code = emitBinary("-");
  else if (command == "and") code = emitBinary("&");
  else if (command == "or" ) code = emitBinary("|");

  else if (command == "neg") code = emitUnary("-");  
  else if (command == "not") code = emitUnary("!");

  else if (command == "eq")  code = emitCompare("EQ");
  else if (command == "gt")  code = emitCompare("GT");
  else if (command == "lt")  code = emitCompare("LT");

  else throw std::invalid_argument("Unknown arithmetic command: " + command);

  m_outputFile << code;
}

void CodeWriter::writePushPop(CommandType command, const std::string& segment, int index) 
{
  std::string code{};

  if      (command == CommandType::C_PUSH) code = emitPush(segment, index);
  else if (command == CommandType::C_POP ) code = emitPop(segment, index);

  else throw std::invalid_argument("writePushPop called with a command that is not C_PUSH or C_POP");

  m_outputFile << code;
}