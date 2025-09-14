#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <initializer_list>
#include "utils/InputFile.h"
#include "utils/TokenType.h"
#include "utils/KeyWords.h"
#include "utils/Identifiers.h"
#include "utils/CompilationError.h"
#include "utils/VM.h"
#include "compiler/JackTokenizer.h"
#include "compiler/CompilationEngine.h"

CompilationEngine::CompilationEngine(InputFile& inputFile, std::ofstream& outputFile)
  : m_fileName(inputFile.fileName + ".jack")
  , m_writer(outputFile)
  , m_tokenizer(inputFile)
{
}

void CompilationEngine::advanceOrError()
{
  if (!m_tokenizer.hasMoreTokens()) 
  {
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), "Unexpected EOF");
  }

  m_tokenizer.advance();
}

bool CompilationEngine::isOperator() 
{
  if (m_tokenizer.tokenType() != TokenType::SYMBOL) return false;
  switch (m_tokenizer.symbol()) {
    case '+': case '-': case '*': case '/':
    case '&': case '|': case '<': case '>': case '=':
      return true;
    default:
      return false;
  }
}

// ------- EXPECT HELPERS -------

void CompilationEngine::expectIdentifier(Identifiers::Category identifierCategory)
{
  if (m_tokenizer.tokenType() != TokenType::IDENTIFIER) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected " + Identifiers::to_string(identifierCategory) + " identifier"
    );
  }
}

void CompilationEngine::expectIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed)
{
  if (m_tokenizer.tokenType() != TokenType::IDENTIFIER) {
    std::string msg = "Expected ";
    bool first = true;
    for (auto id : allowed) {
      if (!first) msg += " or ";
      msg += Identifiers::to_string(id);
      first = false;
    }
    msg += " identifier";
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
  }
}

void CompilationEngine::expectSymbol(char symbol)
{
  if (m_tokenizer.tokenType() != TokenType::SYMBOL || m_tokenizer.symbol() != symbol) 
  {
    std::string msg = "Expected '";
    msg += symbol;
    msg += "' symbol";
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
  }
}

void CompilationEngine::expectKeyword(KeyWords::KeyWord keyWord)
{
  if (m_tokenizer.tokenType() != TokenType::KEYWORD || m_tokenizer.keyWord() != keyWord) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected '" + KeyWords::to_string(keyWord) + "' keyword"
    );
  }
}

void CompilationEngine::expectKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed)
{
    if (m_tokenizer.tokenType() != TokenType::KEYWORD) {
        throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), "Expected a keyword");
    }

    KeyWords::KeyWord curK = m_tokenizer.keyWord();
    if (std::find(allowed.begin(), allowed.end(), curK) == allowed.end()) {
        std::string msg = "Expected ";
        bool first = true;
        for (auto k : allowed) {
            if (!first) msg += " or ";
            msg += "'";
            msg += KeyWords::to_string(k);
            msg += "'";
            first = false;
        }
        msg += " keyword";
        throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
    }
}

void CompilationEngine::expectIntConst()
{
  if (m_tokenizer.tokenType() != TokenType::INT_CONST) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected integer constant"
    );
  }
}

void CompilationEngine::expectStringConst()
{
  if (m_tokenizer.tokenType() != TokenType::STRING_CONST) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected string constant"
    );
  }
}

void CompilationEngine::expectType(bool voidOption /*= false*/)
{
  if (m_tokenizer.tokenType() == TokenType::KEYWORD) {
    auto kw = m_tokenizer.keyWord();
    if (kw == KeyWords::KeyWord::INT ||
        kw == KeyWords::KeyWord::CHAR ||
        kw == KeyWords::KeyWord::BOOLEAN ||
        (voidOption && kw == KeyWords::KeyWord::VOID)) 
    {
        return; // ok
    }
  } 
  else if (m_tokenizer.tokenType() == TokenType::IDENTIFIER) 
  {
    return; // className
  }

  std::string msg = "Expected type ('int', 'char', 'boolean'";
  if (voidOption) msg += ", 'void'";
  msg += " or class name)";
  throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
}

void CompilationEngine::expectOperator()
{
  if (!isOperator()) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected operator ('+' '-' '*' '/' '&' '|' '<' '>' '=')"
    );
  }
}

void CompilationEngine::expectUnaryOperator()
{
  if ((m_tokenizer.tokenType() != TokenType::SYMBOL) ||
      (m_tokenizer.symbol() != '-' && m_tokenizer.symbol() != '~')) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected unary operator ('-' or '~')"
    );
  }
}

// ------- HANDLERS (expect + advance) -------

void CompilationEngine::handleSymbol(char symbol)
{
  expectSymbol(symbol);
  advanceOrError();
}

void CompilationEngine::handleKeyword(KeyWords::KeyWord keyWord)
{
  expectKeyword(keyWord);
  advanceOrError();
}

void CompilationEngine::handleKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed)
{
  expectKeywordOneOf(allowed);
  advanceOrError();
}
// ------- PARSER -------

void CompilationEngine::compileClass()
{
  handleKeyword(KeyWords::KeyWord::CLASS);
  
  expectIdentifier(Identifiers::Category::CLASS);
  m_className = m_tokenizer.identifier();
  advanceOrError();

  handleSymbol('{');

  while (m_tokenizer.tokenType() == TokenType::KEYWORD &&
         (m_tokenizer.keyWord() == KeyWords::KeyWord::FIELD ||
          m_tokenizer.keyWord() == KeyWords::KeyWord::STATIC))
  {
    compileClassVarDec();
  }

  while (m_tokenizer.tokenType() == TokenType::KEYWORD &&
         (m_tokenizer.keyWord() == KeyWords::KeyWord::CONSTRUCTOR ||
          m_tokenizer.keyWord() == KeyWords::KeyWord::FUNCTION ||
          m_tokenizer.keyWord() == KeyWords::KeyWord::METHOD))
  {
    compileSubroutine();
  }

  expectSymbol('}');
  // No advance
}

void CompilationEngine::compileClassVarDec()
{
  expectKeywordOneOf({KeyWords::KeyWord::FIELD, KeyWords::KeyWord::STATIC});
  Identifiers::VarKind kind{ 
    m_tokenizer.keyWord() == KeyWords::KeyWord::FIELD ? Identifiers::VarKind::FIELD : 
                                                        Identifiers::VarKind::STATIC
  };
  advanceOrError();

  expectType();
  std::string type{
    m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                    m_tokenizer.identifier()
  };
  advanceOrError();

  auto declare = [&](const std::string& n)
  {
    try { m_symbolTable.define(n, type, kind); } 
    catch (const std::logic_error& e) {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), e.what());
    }
  };

  expectIdentifier(Identifiers::Category::VAR);
  std::string name{ m_tokenizer.identifier() };
  declare(name);
  advanceOrError();

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');

    expectIdentifier(Identifiers::Category::VAR);
    name = m_tokenizer.identifier();
    declare(name);
    advanceOrError();
  }

  handleSymbol(';');
}

void CompilationEngine::compileSubroutine()
{
  m_symbolTable.startSubroutine();

  expectKeywordOneOf({KeyWords::KeyWord::CONSTRUCTOR, KeyWords::KeyWord::FUNCTION, KeyWords::KeyWord::METHOD});
  bool isMethod{ m_tokenizer.keyWord() == KeyWords::KeyWord::METHOD };
  bool isConstructor{ m_tokenizer.keyWord() == KeyWords::KeyWord::CONSTRUCTOR };

  switch (m_tokenizer.keyWord()) 
  {
  case KeyWords::KeyWord::CONSTRUCTOR: m_currSubKind = Identifiers::SubKind::CONSTRUCTOR; break;
  case KeyWords::KeyWord::FUNCTION:    m_currSubKind = Identifiers::SubKind::FUNCTION;    break;
  case KeyWords::KeyWord::METHOD:      m_currSubKind = Identifiers::SubKind::METHOD;      break;
  default:                             m_currSubKind = Identifiers::SubKind::NONE;        break;
  }
  advanceOrError();

  expectType(true);
  std::string type{
    m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                    m_tokenizer.identifier()
  };
  advanceOrError();

  expectIdentifier(Identifiers::Category::SUBROUTINE);
  std::string name{ m_className + "." + m_tokenizer.identifier() };
  advanceOrError();
  
  handleSymbol('(');

  // ➜ add "this" as ARG 0 for methods
  if (isMethod) {
    try {
      m_symbolTable.define("this", m_className, Identifiers::VarKind::ARG);
    } catch (const std::logic_error& e) {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), e.what());
    }
  }
  
  compileParameterList();
  
  handleSymbol(')');
    
  handleSymbol('{');
  
  while (m_tokenizer.tokenType() == TokenType::KEYWORD &&
         m_tokenizer.keyWord() == KeyWords::KeyWord::VAR)
  {
    compileVarDec();
  }

  m_writer.writeFunction(name, m_symbolTable.varCount(Identifiers::VarKind::VAR));

  if (isMethod) // Set "this" to point to the passed object
  {
    m_writer.writePush(VM::Segment::ARG, 0);
    m_writer.writePop(VM::Segment::POINTER, 0);
  }

  if (isConstructor)
  {
    m_writer.writePush(VM::Segment::CONST, m_symbolTable.varCount(Identifiers::VarKind::FIELD));
    m_writer.writeCall("Memory.alloc", 1);
    m_writer.writePop(VM::Segment::POINTER, 0);
  }

  compileStatements();

  handleSymbol('}');
}

void CompilationEngine::compileParameterList()
{

  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ')') return;

  expectType();
  std::string type{
    m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                    m_tokenizer.identifier()
  };
  advanceOrError();

  auto declare = [&](const std::string& n)
  {
    try { m_symbolTable.define(n, type, Identifiers::VarKind::ARG); } 
    catch (const std::logic_error& e) {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), e.what());
    }
  };

  expectIdentifier(Identifiers::Category::VAR);
  std::string name{ m_tokenizer.identifier() };
  declare(name);
  advanceOrError();

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');

    expectType();
    type =
      m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                      m_tokenizer.identifier();
    advanceOrError();

    expectIdentifier(Identifiers::Category::VAR);
    name = m_tokenizer.identifier();
    declare(name);
    advanceOrError();
  }
}

void CompilationEngine::compileVarDec()
{
  handleKeyword(KeyWords::KeyWord::VAR);

  expectType();
  std::string type{
    m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                    m_tokenizer.identifier()
  };
  advanceOrError();

  auto declare = [&](const std::string& n)
  {
    try { m_symbolTable.define(n, type, Identifiers::VarKind::VAR); } 
    catch (const std::logic_error& e) {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), e.what());
    }
  };

  expectIdentifier(Identifiers::Category::VAR);
  std::string name{ m_tokenizer.identifier() };
  declare(name);
  advanceOrError();

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');

    expectIdentifier(Identifiers::Category::VAR);
    name = m_tokenizer.identifier();
    declare(name);
    advanceOrError();
  }

  handleSymbol(';');

}

void CompilationEngine::compileStatements()
{

  
  while (true) 
  {
    if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '}')
      break;
      
    if (m_tokenizer.tokenType() != TokenType::KEYWORD)
    {
      std::string msg = "Expected statement keyword ('let', 'if', 'while', 'do', 'return')";
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
    }

    switch (m_tokenizer.keyWord())
    {
    case KeyWords::KeyWord::LET:    compileLet();    break;
    case KeyWords::KeyWord::IF:     compileIf();     break;
    case KeyWords::KeyWord::WHILE:  compileWhile();  break;
    case KeyWords::KeyWord::DO:     compileDo();     break;
    case KeyWords::KeyWord::RETURN: compileReturn(); break;

    default: 
    {
      std::string msg = "Expected statement keyword ('let', 'if', 'while', 'do', 'return')";
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
    }
    }
  }
}

void CompilationEngine::compileDo()
{
  handleKeyword(KeyWords::KeyWord::DO);

  // Read first identifier (subroutine | class | var)
  expectIdentifierOneOf({Identifiers::Category::SUBROUTINE,
                         Identifiers::Category::CLASS,
                         Identifiers::Category::VAR});
  std::string baseName = m_tokenizer.identifier();
  advanceOrError();

  std::string callName;
  int nArgs{ 0 };

  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '.') 
  {
    // Case: obj.method(...)  or  Class.function(...)
    handleSymbol('.');

    expectIdentifier(Identifiers::Category::SUBROUTINE);
    std::string subName = m_tokenizer.identifier();
    advanceOrError();

    // If baseName is a variabile → method call on object
    Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };
    if (kind != Identifiers::VarKind::NONE) 
    {
      int index = m_symbolTable.indexOf(baseName);
      std::string type = m_symbolTable.typeOf(baseName);
      // push object as arg0
      m_writer.writePush(segmentOf(kind), index);
      nArgs = 1;
      callName = type + "." + subName;
    } else {
      // otherwise it is the name of a class
      callName = baseName + "." + subName;
    }
  } else {
    // Case: subroutineName(...)  → implicit method on this
    if (m_currSubKind == Identifiers::SubKind::FUNCTION) {
      throw CompilationError(
          m_fileName, m_tokenizer.tokenLineIdx(),
          "Invalid unqualified call '" + baseName +
          "' inside a function; qualify with a class name (e.g. " + m_className + "." + baseName +
          ") or call a method via an object (obj." + baseName + ")"
      );
    }
    m_writer.writePush(VM::Segment::POINTER, 0); // this
    nArgs = 1;
    callName = m_className + "." + baseName;
  }

  handleSymbol('(');
  int exprCount = compileExpressionList();
  handleSymbol(')');

  m_writer.writeCall(callName, nArgs + exprCount);
  // "do" discards the return value
  m_writer.writePop(VM::Segment::TEMP, 0);

  handleSymbol(';');
}

void CompilationEngine::compileLet()
{
  handleKeyword(KeyWords::KeyWord::LET);

  // varName
  expectIdentifier(Identifiers::Category::VAR);
  std::string name{ m_tokenizer.identifier() };

  Identifiers::VarKind kind{ m_symbolTable.kindOf(name) };

  if (kind == Identifiers::VarKind::NONE) 
  {
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                           "Undefined variable '" + name + "' in let statement");
  }

  int index{ m_symbolTable.indexOf(name) };

  VM::Segment baseSeg{ segmentOf(kind) };

  advanceOrError();

  // let a[expr] = expr;
  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '[') 
  {
    handleSymbol('[');
    // push base
    m_writer.writePush(baseSeg, index);
    // push expr
    compileExpression();
    handleSymbol(']');

    handleSymbol('=');

    compileExpression();                        // RHS
    m_writer.writePop(VM::Segment::TEMP, 0);    // save RHS
    m_writer.writeArithmetic(VM::Command::ADD); // sum base+offset
    m_writer.writePop(VM::Segment::POINTER, 1); // THAT = base+offset
    m_writer.writePush(VM::Segment::TEMP, 0);   // restore RHS
    m_writer.writePop(VM::Segment::THAT, 0);    // *THAT = RHS

    handleSymbol(';');
    return;
  }

  // let x = expr;
  handleSymbol('=');
  compileExpression();
  m_writer.writePop(baseSeg, index);
  handleSymbol(';');
}

void CompilationEngine::compileWhile()
{
  handleKeyword(KeyWords::KeyWord::WHILE);
  handleSymbol('(');

  std::string label1{ m_className + "_" + std::to_string(m_labelIdx++) };
  std::string label2{ m_className + "_" + std::to_string(m_labelIdx++) };

  m_writer.writeLabel(label1);

  compileExpression();
  m_writer.writeArithmetic(VM::Command::NOT);

  handleSymbol(')');

  m_writer.writeIf(label2);

  handleSymbol('{');

  compileStatements();

  m_writer.writeGoto(label1);

  handleSymbol('}');

  m_writer.writeLabel(label2);
}

void CompilationEngine::compileReturn()
{
  handleKeyword(KeyWords::KeyWord::RETURN);

  if (m_tokenizer.tokenType() != TokenType::SYMBOL || m_tokenizer.symbol() != ';') {
    compileExpression();
  } else {
    m_writer.writePush(VM::Segment::CONST, 0);
  }

  m_writer.writeReturn();

  handleSymbol(';');
}

void CompilationEngine::compileIf()
{
  handleKeyword(KeyWords::KeyWord::IF);
  handleSymbol('(');

  compileExpression();
  m_writer.writeArithmetic(VM::Command::NOT);

  handleSymbol(')');
  
  std::string label1{ m_className + "_" + std::to_string(m_labelIdx++) };
  std::string label2{ m_className + "_" + std::to_string(m_labelIdx++) };
  
  handleSymbol('{');

  m_writer.writeIf(label1);

  compileStatements();

  m_writer.writeGoto(label2);

  handleSymbol('}');

  m_writer.writeLabel(label1);

  if (m_tokenizer.tokenType() == TokenType::KEYWORD && m_tokenizer.keyWord() == KeyWords::KeyWord::ELSE)
  {
    handleKeyword(KeyWords::KeyWord::ELSE);
    handleSymbol('{');

    compileStatements();

    handleSymbol('}');
  }

  m_writer.writeLabel(label2);
}

int CompilationEngine::compileExpressionList()
{
  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ')') return 0;

  int exprCount{ 0 };

  while (true) 
  {
    compileExpression();
    ++exprCount;

    if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',') {
      handleSymbol(',');  
    } else {
      break; 
    }
  }

  return exprCount;
}

void CompilationEngine::compileExpression()
{
  compileTerm();

  while (isOperator())
  {
    expectOperator();
    char op{ m_tokenizer.symbol() };
    advanceOrError();

    compileTerm();

    if (op == '*') {
      m_writer.writeCall("Math.multiply", 2);
    } else if (op == '/') {
      m_writer.writeCall("Math.divide", 2);
    } else {
      // + - & | < > =
      m_writer.writeArithmetic(VM::command_from_char(op));
    }
  }
}

void CompilationEngine::compileTerm()
{
  // 1) IDENTIFIER: VarName | VarName[expression] | Subroutine call
  if (m_tokenizer.tokenType() == TokenType::IDENTIFIER) 
  {
    expectIdentifierOneOf({Identifiers::Category::SUBROUTINE,
                           Identifiers::Category::CLASS,
                           Identifiers::Category::VAR});
    std::string baseName = m_tokenizer.identifier();
    advanceOrError();
                            
    // Subroutine call: (className | varName).subroutineName(expressionList) 
    if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '.')
    {
      handleSymbol('.');

      expectIdentifier(Identifiers::Category::SUBROUTINE);
      std::string subName = m_tokenizer.identifier();
      advanceOrError();

      std::string callName;
      int nArgs = 0;

      // If baseName is a variabile → method call on object
      Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };
      if (kind != Identifiers::VarKind::NONE) 
      {
        int index = m_symbolTable.indexOf(baseName);
        std::string type = m_symbolTable.typeOf(baseName);
        // push object as arg0
        m_writer.writePush(segmentOf(kind), index);
        nArgs = 1;
        callName = type + "." + subName;
      } else {
        // otherwise it is the name of a class
        callName = baseName + "." + subName;
      }

      handleSymbol('(');
      int exprCount = compileExpressionList();
      handleSymbol(')');

      m_writer.writeCall(callName, nArgs + exprCount);

      return;
    }
    // Subroutine call: subroutineName(expressionList)                      
    if ((m_tokenizer.tokenType() == TokenType::SYMBOL) && (m_tokenizer.symbol() == '('))  
    {
      if (m_currSubKind == Identifiers::SubKind::FUNCTION) {
        throw CompilationError(
          m_fileName, m_tokenizer.tokenLineIdx(),
          "Invalid unqualified call '" + baseName +
          "' inside a function; qualify with a class name (e.g. " + m_className + "." + baseName +
          ") or call a method via an object (obj." + baseName + ")"
        );
      }

      handleSymbol('(');
      m_writer.writePush(VM::Segment::POINTER, 0); // push this as arg0
      int nArgs = 1 + compileExpressionList();
      handleSymbol(')');

      std::string callName = m_className + "." + baseName;
      m_writer.writeCall(callName, nArgs);

      return;
    }                  
    // VarName[expression]
    if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '[')
    {
      Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };
      if (kind != Identifiers::VarKind::NONE) 
      {
        int index = m_symbolTable.indexOf(baseName);

        handleSymbol('[');
        m_writer.writePush(segmentOf(kind), index); // push base address
        compileExpression();                        // push index
        handleSymbol(']');

        m_writer.writeArithmetic(VM::Command::ADD);    // base+index
        m_writer.writePop(VM::Segment::POINTER, 1);    // THAT = base+index
        m_writer.writePush(VM::Segment::THAT, 0);      // push *addr
      } 
      else 
      {
        throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                               "Undefined variable '" + baseName + "'");
      }

      return;
    }
    // Case: simple varName → push value
    {
      Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };
      if (kind != Identifiers::VarKind::NONE)
      {
        int index = m_symbolTable.indexOf(baseName);
        m_writer.writePush(segmentOf(kind), index);
      }
      else 
      {
        throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                               "Undefined variable '" + baseName + "'");
      }

      return;
    }
  }
  // 2) Keyword constant
  else if ((m_tokenizer.tokenType() == TokenType::KEYWORD) &&
           (m_tokenizer.keyWord() == KeyWords::KeyWord::TRUE  ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::FALSE ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::NULL_ ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::THIS  ))
  {
    expectKeywordOneOf({KeyWords::KeyWord::TRUE, KeyWords::KeyWord::FALSE, 
                        KeyWords::KeyWord::NULL_, KeyWords::KeyWord::THIS});
    KeyWords::KeyWord kw{ m_tokenizer.keyWord() };
    switch (kw)
    {
    case KeyWords::KeyWord::TRUE:
    {
      m_writer.writePush(VM::Segment::CONST, 1);
      m_writer.writeArithmetic(VM::Command::NEG);
      break;
    }
    case KeyWords::KeyWord::FALSE:
    case KeyWords::KeyWord::NULL_:
    {
      m_writer.writePush(VM::Segment::CONST, 0);
      break;
    }
    case KeyWords::KeyWord::THIS:
    {
      m_writer.writePush(VM::Segment::POINTER, 0);
      break;
    }
    default: break;
    }
    advanceOrError();
  }
  // 3) Unary op + term
  else if ((m_tokenizer.tokenType() == TokenType::SYMBOL) &&
           (m_tokenizer.symbol() == '-' || m_tokenizer.symbol() == '~'))
  {
    expectUnaryOperator();
    char op{ m_tokenizer.symbol() };
    advanceOrError();

    compileTerm();

    m_writer.writeArithmetic(VM::command_from_char(op, true));
  }
  // 4) ( expression )
  else if ((m_tokenizer.tokenType() == TokenType::SYMBOL) && (m_tokenizer.symbol() == '('))
  {
    handleSymbol('(');

    compileExpression();

    handleSymbol(')');
  }
  // 5) Integer constant
  else if (m_tokenizer.tokenType() == TokenType::INT_CONST)
  {
    expectIntConst();
    m_writer.writePush(VM::Segment::CONST, m_tokenizer.intVal());
    advanceOrError();
  }
  // 6) String constant
  else if (m_tokenizer.tokenType() == TokenType::STRING_CONST)
  {
    expectStringConst();
    std::string str{ m_tokenizer.stringVal() };
    m_writer.writePush(VM::Segment::CONST, static_cast<int>(str.size()));
    m_writer.writeCall("String.new", 1);

    for (const auto c : str) 
    {
      m_writer.writePush(VM::Segment::CONST, static_cast<int>(c));
      m_writer.writeCall("String.appendChar", 2);
    }

    advanceOrError();
  }
  // 7)
  else 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected an identifier, a keyword constant ('true','false','null','this'), "
      "an integer constant, a string constant, '(', '-' or '~'"
    );
  }
}

void CompilationEngine::compile()
{
  advanceOrError();
  compileClass();

  if (m_tokenizer.hasMoreTokens()) 
  {
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                          "Extra tokens after class declaration");
  }
}
