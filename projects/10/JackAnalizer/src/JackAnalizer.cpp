#include <string>
#include <filesystem>
#include <iostream>
#include "InputFile.h"
#include "TokenType.h"
#include "KeyWords.h"
#include "JackAnalizer.h"
#include "JackTokenizer.h"
#include "CompilationEngine.h"

namespace fs = std::filesystem;

JackAnalizer::JackAnalizer(InputFiles& inputFiles, fs::path& outputDir)
  : m_inputFiles(inputFiles)
  , m_outputDir(outputDir)
{
}

void JackAnalizer::analize()
{
  // Iterate over each input file and create its corresponding output files
  for (size_t i = 0; i < m_inputFiles.size(); ++i)
  {
    auto& inputFile = m_inputFiles[i]; // Get the input file

    std::string baseName = inputFile.fileName;
    std::string xmlName = baseName + ".xml";
    std::string xmlTName = baseName + "T.xml";

    std::ofstream xmlOutputFile(m_outputDir / xmlName);
    if (!xmlOutputFile)
    {
      throw std::runtime_error("Unable to create output file: " + xmlName);
    }

    std::ofstream xmlTOutputFile(m_outputDir / xmlTName);
    if (!xmlTOutputFile)
    {
      throw std::runtime_error("Unable to create output file: " + xmlTName);
    }

    // Process the input file and write to the respective output files

    std::cout << inputFile.fileName << ".jack: \n";

    // ----- 1) Tokenizer -----

    JackTokenizer tokenizer(inputFile);

    xmlTOutputFile << "<tokens>\n";

    while (tokenizer.hasMoreTokens())
    {
      tokenizer.advance();

      TokenType tokenType{ tokenizer.tokenType() };
      switch (tokenType)
      {
        case TokenType::KEYWORD:
        {
          xmlTOutputFile << "<keyword> ";
          xmlTOutputFile << KeyWords::to_string(tokenizer.keyWord());
          xmlTOutputFile << " </keyword>\n";
          break;
        }
        case TokenType::SYMBOL:
        {
          std::string symbol{ tokenizer.symbol() };
          if      (symbol == "<") symbol = "&lt;";
          else if (symbol == ">") symbol = "&gt;";
          else if (symbol == "&") symbol = "&amp;";

          xmlTOutputFile << "<symbol> ";
          xmlTOutputFile << symbol;
          xmlTOutputFile << " </symbol>\n";
          break;
        }
        case TokenType::IDENTIFIER:
        {
          xmlTOutputFile << "<identifier> ";
          xmlTOutputFile << tokenizer.identifier();
          xmlTOutputFile << " </identifier>\n";
          break;
        }
        case TokenType::INT_CONST:
        {
          xmlTOutputFile << "<integerConstant> ";
          xmlTOutputFile << tokenizer.intVal();
          xmlTOutputFile << " </integerConstant>\n";
          break;
        }
        case TokenType::STRING_CONST:
        {
          xmlTOutputFile << "<stringConstant> ";
          xmlTOutputFile << tokenizer.stringVal();
          xmlTOutputFile << " </stringConstant>\n";
          break;
        }
        default:
        {
          throw std::logic_error("Unknown token type");
        }        
      }
    }

    xmlTOutputFile << "</tokens>\n";

    std::cout << "Tokenization completed\n";

    // ----- 2) Parser -----

    // Restore the stream and return to the beginning of the file
    inputFile.file.clear();
    inputFile.file.seekg(0, std::ios::beg);

    CompilationEngine compilationEngine(inputFile, xmlOutputFile);

    compilationEngine.compile();
    
    std::cout << "Parsing completed\n\n";
  }
}
