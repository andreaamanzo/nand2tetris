#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <vector>
#include <fstream>
#include "InputFiles.h"

class Translator
{
private:
  InputFiles& m_inputFiles;
  std::ofstream& m_outputFile;

public:
  Translator(InputFiles& inputFiles, std::ofstream& outputFile);
  void translate();
};

#endif