#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include "InputFiles.h"
#include "Translator.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) 
{
  if (argc != 2) 
  {
    std::cerr << "Missing .vm file or directory" << std::endl;
    return 1;
  }

  fs::path inPath(argv[1]);
  if (!fs::exists(inPath)) 
  {
    std::cerr << "Unable to access path: " << inPath << std::endl;
    return 1;
  }

  InputFiles inputFiles{};
  std::string outputFileName{};

  if (fs::is_directory(inPath)) 
  {
    // 1) raccogli i .vm (solo file regolari)
    std::vector<fs::path> vmFiles;
    for (const auto& entry : fs::directory_iterator(inPath)) 
    {
      if (!entry.is_regular_file()) continue;
      std::string ext = entry.path().extension().string(); // ".vm"
      if (ext == ".vm") vmFiles.push_back(entry.path());
    }

    if (vmFiles.empty()) 
    {
      std::cerr << "No .vm files in directory: " << inPath << std::endl;
      return 1;
    }

    // 2) ordine deterministico
    std::sort(vmFiles.begin(), vmFiles.end());

    // 3) apri e inserisci in inputFiles
    for (const auto& p : vmFiles) 
    {
      std::ifstream inputFile(p);
      if (!inputFile) 
      {
        std::cerr << "Unable to open file: " << p << std::endl;
        return 1;
      }
      inputFiles.push_back({
        p.stem().string(),       // nome senza estensione
        std::move(inputFile)     // ifstream è move-only
      });
    }

    // 4) nome output: <dir>/<dir>.asm
    outputFileName = (inPath / (
      (inPath.has_filename() ? inPath.filename() : inPath.parent_path().filename()).string() + ".asm"
    )).string();
  }
  else if (fs::is_regular_file(inPath)) 
  {
    // controllo estensione
    std::string ext = inPath.extension().string();
    if (ext != ".vm") 
    {
      std::cerr << "Input file must have a .vm extension" << std::endl;
      return 1;
    }

    std::ifstream inputFile(inPath);
    if (!inputFile) 
    {
      std::cerr << "Unable to open file: " << inPath << std::endl;
      return 1;
    }

    inputFiles.push_back({
      inPath.stem().string(),   // base name (senza .vm)
      std::move(inputFile)
    });

    // output accanto al file: <file>.asm
    outputFileName =
      (inPath.parent_path() / (inPath.stem().string() + ".asm")).string();
  }
  else 
  {
    std::cerr << inPath << " is neither a file nor a directory." << std::endl;
    return 1;
  }

  // crea il file di output
  std::ofstream outputFile(outputFileName);
  if (!outputFile) 
  {
    std::cerr << "Unable to create output file: " << outputFileName << std::endl;
    return 1;
  }

  Translator translator(inputFiles, outputFile);

  translator.translate();

  std::cout << "Translation completed. Output written to: " << outputFileName << std::endl;

  return 0;
}
