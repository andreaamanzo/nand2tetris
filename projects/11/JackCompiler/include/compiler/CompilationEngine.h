#ifndef COMPILATIONENGINE_H
#define COMPILATIONENGINE_H

#include <fstream>
#include <string>
#include <initializer_list>
#include "utils/InputFile.h"
#include "utils/Identifiers.h"
#include "compiler/JackTokenizer.h"
#include "compiler/VMWriter.h"
#include "compiler/SymbolTable.h"

class CompilationEngine 
{
private:
  std::string m_fileName{};
  VMWriter m_writer;
  JackTokenizer m_tokenizer;
  SymbolTable m_symbolTable{};
  std::string m_className{};
  Identifiers::SubKind m_currSubKind{ Identifiers::SubKind::NONE };
  int m_labelIdx{ 0 };

  static VM::Segment segmentOf(Identifiers::VarKind k)
  {
    switch(k)
    { 
      case Identifiers::VarKind::FIELD:  return VM::Segment::THIS;
      case Identifiers::VarKind::STATIC: return VM::Segment::STATIC;
      case Identifiers::VarKind::VAR:    return VM::Segment::LOCAL;
      case Identifiers::VarKind::ARG:    return VM::Segment::ARG; 
      case Identifiers::VarKind::NONE:
        throw std::logic_error("Invalid VarKind::NONE passed to CompilationEngine::segmentOf()");
    }
    return VM::Segment::LOCAL; // never
  }

  void advanceOrError();
  bool isOperator();

  void expectIdentifier(Identifiers::Category identifierCategory);
  void expectIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed);
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
  int compileExpressionList();
  void compileTerm();

  
public:
  CompilationEngine(InputFile& inputFile, std::ofstream& outputFile);

  void compile();

};

#endif