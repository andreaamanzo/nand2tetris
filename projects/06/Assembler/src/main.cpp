#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "Assembler.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Missing .asm file" << std::endl;
    return 1;
  }

  fs::path inPath(argv[1]);

  if (!fs::exists(inPath)) 
  {
    std::cerr << "Unable to access path: " << inPath << std::endl;
    return 1;
  }

  if (fs::is_regular_file(inPath))
  {
    std::string ext = inPath.extension().string();
    if (ext != ".asm") 
    {
      std::cerr << "Input file must have a .asm extension" << std::endl;
      return 1;
    }
  }
  else
  {
    std::cerr << inPath << " is not a .asm file" << std::endl;
    return 1;
  }

  // input file
  std::ifstream inputFile(argv[1]);
  if (!inputFile) {
    std::cerr << "Unable to open input file: " << argv[1] << std::endl;
    return 1; 
  }

  // output file
  std::string outputFileName = 
    (inPath.parent_path() / (inPath.stem().string() + ".hack")).string();

  std::ofstream outputFile(outputFileName);
  if (!outputFile) 
  {
    std::cerr << "Unable to create output file: " << outputFileName << std::endl;
    return 1;
  }

  Assembler assembler(inputFile, outputFile);

  assembler.generate();

  std::cout << "Binary file (.hack) generated. Output written to: " << outputFileName << std::endl;

  return 0;
}
