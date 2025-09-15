#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

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
  SymbolTable m_symbolTable;
  std::string m_className{};
  Identifiers::SubKind m_currSubKind{ Identifiers::SubKind::NONE };
  bool m_isCurrSubVoid{ false };
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
  bool isOperator() const noexcept;
  void tryDefine(const std::string& name, const std::string& type, Identifiers::VarKind kind);
  std::string getNewLabel();

  void expectIdentifier(Identifiers::Category identifierCategory) const;
  void expectIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed) const;
  void expectSymbol(char symbol) const;
  void expectKeyword(KeyWords::KeyWord keyWord) const;
  void expectKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed) const;
  void expectIntConst() const;
  void expectStringConst() const;
  void expectType(bool voidOption = false) const;
  void expectOperator() const;
  void expectUnaryOperator() const;

  void handleSymbol(char symbol);
  KeyWords::KeyWord handleKeyword(KeyWords::KeyWord keyWord);
  KeyWords::KeyWord handleKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed);
  std::string handleIdentifier(Identifiers::Category identifierCategory);
  std::string handleIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed);
  int handleIntConst();
  std::string handleStringConst();
  std::string handleType(bool voidOption = false);
  char handleOperator();
  char handleUnaryOperator();

  void compileClass();
  void compileClassVarDec();
  void compileSubroutine();
  void compileParameterList();
  void compileVarDec();
  void compileStatements();
  void compileSubroutineCall(const std::string& inBaseName = "");
  void compileDo();
  void compileLet();
  void compileWhile();
  void compileIf();
  void compileReturn();
  void compileExpression();
  int  compileExpressionList();
  void compileTerm();
  
public:
  CompilationEngine(InputFile& inputFile, std::ofstream& outputFile);

  void compile();
};

#endif