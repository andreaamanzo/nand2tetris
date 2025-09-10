#ifndef IOFILES_H
#define IOFILES_H

#include <fstream>
#include <string>
#include <vector>

struct InputFile
{
  std::string fileName;
  std::ifstream file;
};

struct OutputFile
{
  std::string fileName;
  std::ofstream file;
};

typedef std::vector<InputFile> InputFiles;
typedef std::vector<OutputFile> OutputFiles;

#endif