#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <fstream>
#include <string>
#include "InputFiles.h"

class Translator
{
private:
  InputFiles& m_inputFiles;
  std::ofstream& m_outputFile;
  std::string m_currentFunctionName{ };

  static bool isValidName(const std::string& name)
  {
    bool isValid{ 
      name.size() > 0 &&
      !std::isdigit(name[0]) && 
      std::all_of(name.begin(), name.end(), [](char c) {
        return std::isdigit(c) || std::isalpha(c) || c == '.' || c == ':' || c == '_';
      })
    };

    return isValid;
  }

public:
  Translator(InputFiles& inputFiles, std::ofstream& outputFile);

  void translate();
};

#endif