#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <filesystem>
#include <algorithm>
#include "IOFiles.h"
#include "JackAnalizer.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) 
{
  if (argc != 2) 
  {
    std::cerr << "Missing .jack file or directory" << std::endl;
    return 1;
  }

  fs::path inPath(argv[1]);
  if (!fs::exists(inPath)) 
  {
    std::cerr << "Unable to access path: " << inPath << std::endl;
    return 1;
  }

  InputFiles inputFiles{};
  fs::path outputDir;

  // ---- DIRECTORY ----
  if (fs::is_directory(inPath)) 
  {
    // 1) Collect all .jack files recursively (only regular files)
    std::vector<fs::path> jackFiles;
    for (const auto& entry :
         fs::recursive_directory_iterator(inPath, fs::directory_options::skip_permission_denied))
    {
      if (!entry.is_regular_file()) continue;
      if (entry.path().extension() == ".jack") jackFiles.push_back(entry.path());
    }

    if (jackFiles.empty()) 
    {
      std::cerr << "No .jack files in directory (recursively): " << inPath << std::endl;
      return 1;
    }

    // 2) Sort in deterministic order (lexicographically)
    std::sort(jackFiles.begin(), jackFiles.end(),
              [](const fs::path& a, const fs::path& b){
                return a.generic_string() < b.generic_string();
              });

    // 3) Open each file and insert it into inputFiles
    for (const auto& p : jackFiles) 
    {
      std::ifstream inputFile(p);
      if (!inputFile) 
      {
        std::cerr << "Unable to open file: " << p << std::endl;
        return 1;
      }
      inputFiles.push_back({
        p.stem().string(),       // file name without extension
        std::move(inputFile)     // ifstream is move-only
      });
    }

    // 4) Set the output directory to "out_analizer" inside the input directory
    outputDir = inPath / "out_analizer";
    if (!fs::exists(outputDir)) 
    {
      fs::create_directory(outputDir);
    }

  }
  // ---- SINGLE FILE ----
  else if (fs::is_regular_file(inPath)) 
  {
    // 1) Check file extension
    if (inPath.extension() != ".jack") 
    {
      std::cerr << "Input file must have a .jack extension" << std::endl;
      return 1;
    }

    // 2) No sorting is needed

    // 3) Open the file and insert it into inputFiles
    std::ifstream inputFile(inPath);
    if (!inputFile) 
    {
      std::cerr << "Unable to open file: " << inPath << std::endl;
      return 1;
    }

    inputFiles.push_back({
      inPath.stem().string(),   // base name (without .jack)
      std::move(inputFile)
    });

    // 4) Set output directory to "out_analizer" at the same level as the input file
    outputDir = inPath.parent_path() / "out_analizer";
    if (!fs::exists(outputDir)) 
    {
      fs::create_directory(outputDir);
    }
  }
  // ---- NEITHER A FILE NOR A DIRECTORY ----
  else 
  {
    std::cerr << inPath << " is neither a file nor a directory." << std::endl;
    return 1;
  }

  // 5) Process the files using JackAnalyzer

  JackAnalizer jackAnalizer(inputFiles, outputDir);

  try
  {
    jackAnalizer.analize();
  }
  catch (const std::runtime_error& e) 
  {
    std::cerr << "Compilation failed:\n  " << e.what() << std::endl;
    return 1;
  }

  std::cout << "Analisys complete. Output files have been created in directory: " 
            << outputDir << std::endl;
  
  return 0;
}
