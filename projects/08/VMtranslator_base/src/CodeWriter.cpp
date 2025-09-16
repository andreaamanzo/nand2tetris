#include <vector>
#include <fstream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "CommandType.h"
#include "CodeWriter.h"

CodeWriter::CodeWriter(std::ofstream& outputFile)
  : m_outputFile(outputFile)
{
}

std::string CodeWriter::uniqueLabelJmp(const std::string& jmp) 
{
  int* ctr =
    (jmp == "EQ") ? &m_eqLabelId :
    (jmp == "GT") ? &m_gtLabelId :
    (jmp == "LT") ? &m_ltLabelId : nullptr;

  if (!ctr)
    throw std::invalid_argument("Unknown jump type: " + jmp);

  int id{ (*ctr)++ }; 
  return jmp + "_END_" + std::to_string(id);
}

std::string CodeWriter::uniqueLabelRetAddress() 
{
  int id{ m_returnAddressId++ };
  return "RETURN_ADDRESS_" + std::to_string(id);
}

std::string CodeWriter::emitBinary(const std::string& op) 
{
  // pop y in D, punta a x e applica: x = x op y
  std::string last{ (op == "-" ? ("M=M" + op + "D\n") : ("M=D" + op + "M\n")) };
  
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
  const std::string label{ uniqueLabelJmp(jmp) };
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

std::string CodeWriter::emitPush(const std::string& value, bool pushAddress)
{
  std::string code =
    "@" + value + "\n" +
    (pushAddress ? "D=A" : "D=M") + "\n"
    "@SP\n"
    "AM=M+1\n"
    "A=A-1\n"
    "M=D\n";

  return code;
}

void CodeWriter::setFileName(const std::string& fileName)
{
  m_fileName = fileName;
}

void CodeWriter::writeInit()
{
  std::string code =
    // SP = 256
    "@256\n"
    "D=A\n"
    "@SP\n"
    "M=D\n";
    
  m_outputFile << code;

  // call Sys.init
  writeCall("Sys.init", 0);
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

  if (command == CommandType::C_PUSH)
  {
    code = emitMemorySegment(segment, index);
    if (segment == "constant") 
      code += "D=A\n";
    else
      code += "D=M\n";

    code +=
      "@SP\n"
      "AM=M+1\n"
      "A=A-1\n"
      "M=D\n";
  }
  else if (command == CommandType::C_POP )
  {
    if (segment == "constant")
      throw std::invalid_argument("Cannot pop to constant");

    // Calcola l’indirizzo target e lo salva in R13
    code = emitMemorySegment(segment, index);
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
  }

  else throw std::invalid_argument("writePushPop called with a command that is not C_PUSH or C_POP");

  m_outputFile << code;
}

void CodeWriter::writeLabel(const std::string& label)
{
  std::string code{ "(" + label + ")\n" };

  m_outputFile << code;
}

void CodeWriter::writeGoto(const std::string& label)
{
  std::string code = 
    "@" + label + "\n"
    "0;JMP\n";
 
  m_outputFile << code;
}

void CodeWriter::writeIf(const std::string& label)
{
  std::string code = 
    // pop dello stack in D
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"

    // jump se il valore != 0
    "@" + label + "\n"
    "D;JNE\n";
 
  m_outputFile << code;
}

void CodeWriter::writeCall(const std::string& functionName, int numArgs)
{
  std::string returnAddressLabel{ uniqueLabelRetAddress() };


  std::string code =
    emitPush(returnAddressLabel, true) +
    emitPush("LCL") +
    emitPush("ARG") +
    emitPush("THIS") +
    emitPush("THAT") +

    // ARG = SP - numArgs - 5
    "@SP\n"
    "D=M\n"
    "@5\n"
    "D=D-A\n"
    "@" + std::to_string(numArgs) + "\n"
    "D=D-A\n"
    "@ARG\n"
    "M=D\n"

    // LCL = SP
    "@SP\n"
    "D=M\n"
    "@LCL\n"
    "M=D\n"

    // goto functionName
    "@" + functionName + "\n"
    "0;JMP\n"

    // return-address label
    "(" + returnAddressLabel + ")\n";

    m_outputFile << code;
}

void CodeWriter::writeFunction(const std::string& functionName, int nLocals)
{
  std::string code =
    // declare a label for the function entry
    "(" + functionName + ")\n";
  
  // initialize all local variables to 0
  for (int i = 0; i < nLocals; i++)
  {
    code +=
      "@SP\n"
      "A=M\n"
      "M=0\n"
      "@SP\n"
      "M=M+1\n";
  }

  m_outputFile << code;
}

void CodeWriter::writeReturn()
{
  auto emitRestore = [](int n, const std::string& symbol) -> std::string
  {
    return 
      "@" + std::to_string(n) + "\n"
      "D=A\n"
      "@LCL\n"
      "A=M-D\n"
      "D=M\n"
      "@" + symbol + "\n"
      "M=D\n";
  };

  std::string code =
    // 1. Pop the return address 
    "@LCL\n"
    "D=M\n"
    "@5\n"    // The return address is stored at LCL - 5
    "A=D-A\n"
    "D=M\n"   // D = return address (the value stored at LCL-5)
    "@R14\n"  // RET
    "M=D\n"

    // 2. Pop the return value
    "@SP\n"
    "AM=M-1\n"
    "D=M\n"
    "@ARG\n"
    "A=M\n"
    "M=D\n"

    // 3. Restore the stack pointer (SP = ARG + 1)
    "@ARG\n"
    "D=M+1\n"
    "@SP\n"
    "M=D\n"

    // 4. Restore THAT, THIS, ARG, LCL
    +  
    emitRestore(1, "THAT") + // THAT is stored at LCL - 1
    emitRestore(2, "THIS") + // THIS is stored at LCL - 2
    emitRestore(3, "ARG")  + // ARG is stored at LCL - 3
    emitRestore(4, "LCL")  + // LCL is stored at LCL - 4

    // 5. Return the saved return address
    "@R14\n"
    "A=M\n"
    "0;JMP\n";

    m_outputFile << code;
}

