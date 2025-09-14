#ifndef VM_WRITER_H
#define VM_WRITER_H

#include <fstream>
#include <string>
#include "utils/VM.h"

class VMWriter
{
private:
  std::ostream& m_outputFile;

public:
  VMWriter(std::ostream& outputFile);

  void writePush(VM::Segment segment, int index);
  void writePop(VM::Segment segment, int index);
  void writeArithmetic(VM::Command command);
  void writeLabel(const std::string& label);
  void writeGoto(const std::string& label);
  void writeIf(const std::string& label);
  void writeCall(const std::string& name, int nArgs);
  void writeFunction(const std::string& name, int nLocals);
  void writeReturn();
};

#endif