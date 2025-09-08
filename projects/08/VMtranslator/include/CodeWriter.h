#ifndef CODEWRITER_H
#define CODEWRITER_H

#include <vector>
#include <fstream>
#include <string>
#include <map>
#include "CommandType.h"

class CodeWriter
{
private:
  std::ofstream& m_outputFile;
  std::string m_fileName{};

  int m_eqLabelId{ 0 };
  int m_gtLabelId{ 0 };
  int m_ltLabelId{ 0 };
  int m_returnAddressId{ 0 };

  std::string uniqueLabelJmp(const std::string& jmp);
  std::string uniqueLabelRetAddress();
  std::string emitBinary(const std::string& op);
  std::string emitUnary(const std::string& op);
  std::string emitCompare(const std::string& jmp);
  std::string emitPush(const std::string& value, bool pushAddress=false);
  std::string emitMemorySegment(const std::string& segment, int index);
  
public:
  CodeWriter(std::ofstream& outputFile);

  void setFileName(const std::string& fileName);
  void writeInit();
  void writeArithmetic(const std::string& command);
  void writePushPop(CommandType command, const std::string& segment, int index);
  void writeLabel(const std::string& label);
  void writeGoto(const std::string& label);
  void writeIf(const std::string& label);
  void writeCall(const std::string functionName, int numArgs);
  void writeFunction(const std::string functionName, int nLocals);
  void writeReturn();
};

#endif