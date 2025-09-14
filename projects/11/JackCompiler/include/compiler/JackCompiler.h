#ifndef JACKANALIZER_H
#define JACKANALIZER_H

#include <string>
#include <filesystem>
#include "utils/InputFile.h"

namespace fs = std::filesystem;

class JackCompiler 
{
private:
  InputFiles& m_inputFiles;
  fs::path& m_outputDir;

public:
  JackCompiler(InputFiles& inputFiles, fs::path& outputDir);

  void compile();
};

#endif