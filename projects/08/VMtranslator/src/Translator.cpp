#include <vector>
#include <fstream>
#include <algorithm>
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

bool Translator::isValidName(const std::string& name)
{
  bool isValid = 
    name.size() > 0 &&
    !std::isdigit(name[0]) && 
    std::all_of(name.begin(), name.end(), [](char c) {
      return std::isdigit(c) || std::isalpha(c) || c == '.' || c == ':' || c == '_';
    });

  return isValid;
}

void Translator::translate()
{
  CodeWriter codeWriter(m_outputFile);

  codeWriter.writeInit();

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
        std::string label{ parser.arg1() };
        if (!isValidName(label))
          throw std::invalid_argument("Invalid label command: invalid label: " + label);

        std::string uniqueLabel{ m_currentFunctionName + "$" + label };
        codeWriter.writeLabel(uniqueLabel);
        break;
      }
      case CommandType::C_GOTO:
      {
        std::string label{ parser.arg1() };
        if (!isValidName(label))
          throw std::invalid_argument("Invalid goto command: invalid label: " + label);

        std::string uniqueLabel{ m_currentFunctionName + "$" + label };
        codeWriter.writeGoto(uniqueLabel);
        break;
      }
      case CommandType::C_IF:
      {
        std::string label{ parser.arg1() };
        if (!isValidName(label))
          throw std::invalid_argument("Invalid if command: invalid label: " + label);

        std::string uniqueLabel{ m_currentFunctionName + "$" + label };
        codeWriter.writeIf(uniqueLabel);
        break;
      }
      case CommandType::C_FUNCTION:
      {
        std::string functionName{ parser.arg1() };
        if (!isValidName(functionName))
          throw std::invalid_argument("Invalid function command: invalid function name: " + functionName);

        int nLocals{ parser.arg2() };
        if (nLocals < 0)
          throw std::invalid_argument("Invalid function command: invalid local variables number: " + nLocals);

        m_currentFunctionName = functionName;

        codeWriter.writeFunction(functionName, nLocals);
        break;
      }
      case CommandType::C_RETURN:
      {
        codeWriter.writeReturn();
        break;
      }
      case CommandType::C_CALL:
      {
        std::string functionName{ parser.arg1() };
        if (!isValidName(functionName))
          throw std::invalid_argument("Invalid call command: invalid function name: " + functionName);
        
        int nArgs{ parser.arg2() };
        if (nArgs < 0)
          throw std::invalid_argument("Invalid call command: invalid arguments number: " + nArgs);

        codeWriter.writeCall(functionName, nArgs);
        break;
      }
      default:
        throw std::runtime_error("Error: unknown command type");
      }
    }
  }
}