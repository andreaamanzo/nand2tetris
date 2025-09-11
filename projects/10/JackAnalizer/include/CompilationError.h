#ifndef COMPILATIONERROR_H
#define COMPILATIONERROR_H

#include <stdexcept>
#include <string>

class CompilationError : public std::runtime_error 
{
private:
  std::string buildMessage(const std::string& fileName,
                           int lineIdx,
                           const std::string& message);

public:
  CompilationError(const std::string& fileName,
                    int lineIdx,
                    const std::string& message);
};

#endif
