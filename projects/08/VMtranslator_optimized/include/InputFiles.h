#ifndef INPUTFILES_H
#define INPUTFILES_H

#include <fstream>
#include <string>
#include <vector>

struct InputFile
{
  std::string fileName;
  std::ifstream file;
};

typedef std::vector<InputFile> InputFiles;

#endif