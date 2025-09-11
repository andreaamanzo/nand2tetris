#ifndef JACKANALIZER_H
#define JACKANALIZER_H

#include <string>
#include <filesystem>
#include "InputFile.h"

namespace fs = std::filesystem;

class JackAnalizer 
{
private:
  InputFiles& m_inputFiles;
  fs::path& m_outputDir;

public:
  JackAnalizer(InputFiles& inputFiles, fs::path& outputDir);

  void analize();
};

#endif