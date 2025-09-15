#include <string>
#include <filesystem>
#include <iostream>
#include "utils/InputFile.h"
#include "compiler/CompilationEngine.h"
#include "compiler/JackCompiler.h"

namespace fs = std::filesystem;

JackCompiler::JackCompiler(InputFiles& inputFiles, fs::path& outputDir)
  : m_inputFiles(inputFiles)
  , m_outputDir(outputDir)
{
}

void JackCompiler::compile()
{
  // Iterate over each input file and create its corresponding output files
  for (size_t i = 0; i < m_inputFiles.size(); ++i)
  {
    auto& inputFile = m_inputFiles[i]; // Get the input file

    std::string outFileName{ inputFile.fileName + ".vm" };

    std::ofstream outputFile(m_outputDir / outFileName);
    if (!outputFile)
    {
      throw std::runtime_error("Unable to create output file: " + outFileName);
    }

    // Process the input file and write to the respective output file
    
    CompilationEngine compilationEngine(inputFile, outputFile);

    compilationEngine.compile();
    
    std::cout << inputFile.fileName << ".jack: " << "Compiled\n";
  }
}
