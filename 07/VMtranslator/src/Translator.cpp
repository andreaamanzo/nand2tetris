#include <vector>
#include <fstream>
#include "Translator.h"
#include "Parser.h"
#include "CodeWriter.h"
#include "CommandType.h"
#include "InputFiles.h"

Translator::Translator(InputFiles& inputFiles, std::ofstream& outputFile)
  : m_inputFiles(inputFiles)
  , m_outputFile(outputFile)
{
}

void Translator::translate()
{
  CodeWriter codeWriter(m_outputFile);

  for (auto& [fileName, file] : m_inputFiles)
  {
    Parser parser(file);
    codeWriter.setFileName(fileName);

    while (parser.hasMoreCommands())
    {
      parser.advance();

      CommandType type{ parser.commandType() };
      switch (type)
      {
      case CommandType::C_ARITHMETIC:
      {
        codeWriter.writeArithmetic(parser.arg1());
        break;
      }
      case CommandType::C_PUSH:
      case CommandType::C_POP:
      {
        codeWriter.writePushPop(type, parser.arg1(), parser.arg2());
        break;
      }
      case CommandType::C_LABEL:
      {
        throw std::runtime_error("Error: label command not implemented");
        break;
      }
      case CommandType::C_GOTO:
      {
        throw std::runtime_error("Error: goto command not implemented");
        break;
      }
      case CommandType::C_IF:
      {
        throw std::runtime_error("Error: if-goto command not implemented");
        break;
      }
      case CommandType::C_FUNCTION:
      {
        throw std::runtime_error("Error: function command not implemented");
        break;
      }
      case CommandType::C_RETURN:
      {
        throw std::runtime_error("Error: return command not implemented");
        break;
      }
      case CommandType::C_CALL:
      {
        throw std::runtime_error("Error: call command not implemented");
        break;
      }
      default:
        throw std::runtime_error("Error: unknown command type");
      }
    }
  }
}