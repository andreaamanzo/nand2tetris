#ifndef COMPILATIONENGINE_H
#define COMPILATIONENGINE_H

#include <fstream>
#include <string>
#include <initializer_list>
#include "InputFile.h"
#include "JackTokenizer.h"
#include "Identifiers.h"
#include "CompilationError.h"

class CompilationEngine 
{
private:
  std::string m_fileName{};
  JackTokenizer m_tokenizer;
  std::ofstream& m_outputFile;

  void advanceOrError();
  bool isOperator();

  void expectIdentifier(Identifiers::Identifier identifier);
  void expectIdentifierOneOf(std::initializer_list<Identifiers::Identifier> allowed);
  void expectSymbol(char symbol);
  void expectKeyword(KeyWords::KeyWord keyWord);
  void expectKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed);
  void expectIntConst();
  void expectStringConst();
  void expectType(bool voidOption = false);
  void expectOperator();
  void expectUnaryOperator();

  void handleSymbol(char symbol);
  void handleKeyword(KeyWords::KeyWord keyWord);
  void handleKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed);
  void handleIdentifier(Identifiers::Identifier identifier);
  void handleIdentifierOneOf(std::initializer_list<Identifiers::Identifier> allowed);
  void handleIntConst();
  void handleStringConst();
  void handleType(bool voidOption = false);
  void handleOperator();
  void handleUnaryOperator();

  void compileClass();
  void compileClassVarDec();
  void compileSubroutine();
  void compileParameterList();
  void compileVarDec();
  void compileStatements();
  void compileDo();
  void compileLet();
  void compileWhile();
  void compileIf();
  void compileReturn();
  void compileExpression();
  void compileExpressionList();
  void compileTerm();

  
public:
  CompilationEngine(InputFile& inputFile, std::ofstream& outputFile);

  void compile();

};

#endif