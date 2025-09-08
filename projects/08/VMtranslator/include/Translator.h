#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <vector>
#include <fstream>
#include <string>
#include "InputFiles.h"

class Translator
{
private:
  InputFiles& m_inputFiles;
  std::ofstream& m_outputFile;
  std::string m_currentFunctionName{ "auto" };

  bool isValidName(const std::string& label);

public:
  Translator(InputFiles& inputFiles, std::ofstream& outputFile);
  void translate();
};

#endif