#include <string>
#include "utils/VM.h"
#include "compiler/VMWriter.h"

VMWriter::VMWriter(std::ostream& outputFile)
  : m_outputFile(outputFile)
{
}

void VMWriter::writePush(VM::Segment segment, int index)
{
  m_outputFile << "push " << VM::to_string(segment) << " " << index << '\n';
}

void VMWriter::writePop(VM::Segment segment, int index)
{
  m_outputFile << "pop " << VM::to_string(segment) << " " << index << '\n';
}

void VMWriter::writeArithmetic(VM::Command command)
{
  m_outputFile << VM::to_string(command) << '\n';
}

void VMWriter::writeLabel(const std::string& label)
{
  m_outputFile << "label " << label << '\n';
}

void VMWriter::writeGoto(const std::string& label)
{
  m_outputFile << "goto " << label << '\n';
}

void VMWriter::writeIf(const std::string& label)
{
  m_outputFile << "if-goto " << label << '\n';
}

void VMWriter::writeCall(const std::string& name, int nArgs)
{
  m_outputFile << "call " << name << " " << nArgs << '\n';
}

void VMWriter::writeFunction(const std::string& name, int nLocals)
{
  m_outputFile << "function " << name << " " << nLocals << '\n';
}

void VMWriter::writeReturn()
{
  m_outputFile << "return\n";
}