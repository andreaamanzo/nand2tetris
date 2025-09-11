#include <string>
#include "CompilationError.h"

CompilationError::CompilationError(const std::string& fileName,
                                   int lineIdx,
                                   const std::string& message)
  : std::runtime_error(buildMessage(fileName, lineIdx, message))
{
}

std::string CompilationError::buildMessage(const std::string& fileName,
                                           int lineIdx,
                                           const std::string& message)
{
  return "Error in file " + fileName +
         " at line " + std::to_string(lineIdx) +
         ": " + message;
}
