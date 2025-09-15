#include <fstream>
#include <string>
#include <algorithm>
#include <initializer_list>
#include "utils/InputFile.h"
#include "utils/TokenType.h"
#include "utils/KeyWords.h"
#include "utils/Identifiers.h"
#include "utils/CompilationError.h"
#include "utils/VM.h"
#include "compiler/JackTokenizer.h"
#include "compiler/SymbolTable.h"
#include "compiler/VMWriter.h"
#include "compiler/CompilationEngine.h"

CompilationEngine::CompilationEngine(InputFile& inputFile, std::ofstream& outputFile)
  : m_fileName(inputFile.fileName + ".jack")
  , m_writer(outputFile)
  , m_tokenizer(inputFile)
  , m_symbolTable()
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

bool CompilationEngine::isOperator() const noexcept
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

void CompilationEngine::tryDefine(const std::string& name, const std::string& type, Identifiers::VarKind kind)
{
  try 
  { 
    m_symbolTable.define(name, type, kind); 
  } 
  catch (const std::logic_error& e) 
  {
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), e.what());
  }
}

std::string CompilationEngine::getNewLabel()
{
  return (m_className + "_" + std::to_string(m_labelIdx++));
}

// ------- EXPECT HELPERS -------

void CompilationEngine::expectIdentifier(Identifiers::Category identifierCategory) const
{
  if (m_tokenizer.tokenType() != TokenType::IDENTIFIER) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected " + Identifiers::to_string(identifierCategory) + " identifier"
    );
  }
}

void CompilationEngine::expectIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed) const
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

void CompilationEngine::expectSymbol(char symbol) const
{
  if (m_tokenizer.tokenType() != TokenType::SYMBOL || m_tokenizer.symbol() != symbol) 
  {
    std::string msg = "Expected '";
    msg += symbol;
    msg += "' symbol";
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
  }
}

void CompilationEngine::expectKeyword(KeyWords::KeyWord keyWord) const
{
  if (m_tokenizer.tokenType() != TokenType::KEYWORD || m_tokenizer.keyWord() != keyWord) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected '" + KeyWords::to_string(keyWord) + "' keyword"
    );
  }
}

void CompilationEngine::expectKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed) const
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

void CompilationEngine::expectIntConst() const
{
  if (m_tokenizer.tokenType() != TokenType::INT_CONST) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected integer constant"
    );
  }
}

void CompilationEngine::expectStringConst() const
{
  if (m_tokenizer.tokenType() != TokenType::STRING_CONST) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected string constant"
    );
  }
}

void CompilationEngine::expectType(bool voidOption /*= false*/) const
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

void CompilationEngine::expectOperator() const
{
  if (!isOperator()) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected operator ('+' '-' '*' '/' '&' '|' '<' '>' '=')"
    );
  }
}

void CompilationEngine::expectUnaryOperator() const
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

KeyWords::KeyWord CompilationEngine::handleKeyword(KeyWords::KeyWord keyWord)
{
  expectKeyword(keyWord);
  KeyWords::KeyWord kw{ m_tokenizer.keyWord() };
  advanceOrError();
  return kw;
}

KeyWords::KeyWord CompilationEngine::handleKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed)
{
  expectKeywordOneOf(allowed);
  KeyWords::KeyWord kw{ m_tokenizer.keyWord() };
  advanceOrError();
  return kw;
}

std::string CompilationEngine::handleIdentifier(Identifiers::Category identifierCategory)
{
  expectIdentifier(identifierCategory);
  std::string name{ m_tokenizer.identifier() };
  advanceOrError();
  return name;
}

std::string CompilationEngine::handleIdentifierOneOf(std::initializer_list<Identifiers::Category> allowed)
{
  expectIdentifierOneOf(allowed);
  std::string name{ m_tokenizer.identifier() };
  advanceOrError();
  return name;
}

int CompilationEngine::handleIntConst()
{
  expectIntConst();
  int val{ m_tokenizer.intVal() };
  advanceOrError();
  return val;
}

std::string CompilationEngine::handleStringConst()
{
  expectStringConst();
  std::string s{ m_tokenizer.stringVal() };
  advanceOrError();
  return s;
}

std::string CompilationEngine::handleType(bool voidOption /*= false*/)
{
  expectType(voidOption);
  std::string type{
    m_tokenizer.tokenType() == TokenType::KEYWORD ? KeyWords::to_string(m_tokenizer.keyWord()) :
                                                    m_tokenizer.identifier()
  };
  advanceOrError();
  return type;
}

char CompilationEngine::handleOperator()
{
  expectOperator();
  char op{ m_tokenizer.symbol() };
  advanceOrError();
  return op;
}

char CompilationEngine::handleUnaryOperator()
{
  expectUnaryOperator();
  char op{ m_tokenizer.symbol() };
  advanceOrError();
  return op;
}

// ------- PARSER -------

void CompilationEngine::compileClass()
{
  handleKeyword(KeyWords::KeyWord::CLASS);
  
  m_className = handleIdentifier(Identifiers::Category::CLASS);

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
  KeyWords::KeyWord kw{ 
    handleKeywordOneOf({KeyWords::KeyWord::FIELD, KeyWords::KeyWord::STATIC}) 
  };
  Identifiers::VarKind kind{ 
    kw == KeyWords::KeyWord::FIELD ? Identifiers::VarKind::FIELD : 
                                     Identifiers::VarKind::STATIC
  };
  std::string type{ handleType() };
  std::string name{ handleIdentifier(Identifiers::Category::VAR) };
  
  tryDefine(name, type, kind);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    name = handleIdentifier(Identifiers::Category::VAR);
    tryDefine(name, type, kind);
  }

  handleSymbol(';');
}

void CompilationEngine::compileSubroutine()
{
  m_symbolTable.startSubroutine();
  m_isCurrSubVoid = false;

  KeyWords::KeyWord kw{
    handleKeywordOneOf({KeyWords::KeyWord::CONSTRUCTOR, KeyWords::KeyWord::FUNCTION, KeyWords::KeyWord::METHOD})
  };
  bool isMethod{ kw == KeyWords::KeyWord::METHOD };
  bool isConstructor{ kw == KeyWords::KeyWord::CONSTRUCTOR };

  switch (kw) 
  {
  case KeyWords::KeyWord::CONSTRUCTOR: m_currSubKind = Identifiers::SubKind::CONSTRUCTOR; break;
  case KeyWords::KeyWord::FUNCTION:    m_currSubKind = Identifiers::SubKind::FUNCTION;    break;
  case KeyWords::KeyWord::METHOD:      m_currSubKind = Identifiers::SubKind::METHOD;      break;
  default:                             m_currSubKind = Identifiers::SubKind::NONE;        break;
  }

  std::string type{ handleType(true) };
  if (type == "void") m_isCurrSubVoid = true;
  std::string name{ m_className + "." + handleIdentifier(Identifiers::Category::SUBROUTINE) };
  
  handleSymbol('(');

  if (isMethod) // Add "this" as ARG 0 for methods
  {
    tryDefine("this", m_className, Identifiers::VarKind::ARG);
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

  std::string type{ handleType() };
  std::string name{ handleIdentifier(Identifiers::Category::VAR) };
  tryDefine(name, type, Identifiers::VarKind::ARG);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    type = handleType();
    name = handleIdentifier(Identifiers::Category::VAR);
    tryDefine(name, type, Identifiers::VarKind::ARG);
  }
}

void CompilationEngine::compileVarDec()
{
  handleKeyword(KeyWords::KeyWord::VAR);

  std::string type{ handleType() };
  std::string name{ handleIdentifier(Identifiers::Category::VAR) };
  tryDefine(name, type, Identifiers::VarKind::VAR);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    name = handleIdentifier(Identifiers::Category::VAR);
    tryDefine(name, type, Identifiers::VarKind::VAR);
  }

  handleSymbol(';');
}

void CompilationEngine::compileStatements()
{
  while (!(m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '}')) 
  {   
    bool error{ false };
    if (m_tokenizer.tokenType() != TokenType::KEYWORD) error = true;
    else
    {
      switch (m_tokenizer.keyWord())
      {
      case KeyWords::KeyWord::LET:    compileLet();    break;
      case KeyWords::KeyWord::IF:     compileIf();     break;
      case KeyWords::KeyWord::WHILE:  compileWhile();  break;
      case KeyWords::KeyWord::DO:     compileDo();     break;
      case KeyWords::KeyWord::RETURN: compileReturn(); break;
      default:                        error = true;    break;
      }
    }

    if(error)
    {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), 
                             "Expected statement keyword ('let', 'if', 'while', 'do', 'return')");
    }
  }
}

void CompilationEngine::compileSubroutineCall(const std::string& inBaseName /*= ""*/)
{
  std::string baseName{ inBaseName };
  if (baseName == "")
  {
    // Read first identifier (subroutine | class | var)
    baseName = handleIdentifierOneOf({ Identifiers::Category::SUBROUTINE,
                                                  Identifiers::Category::CLASS,
                                                  Identifiers::Category::VAR });
  }
  std::string callName{};
  int nArgs{ 0 };

  // Case: obj.method(...)  or  Class.function(...)
  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '.') 
  {
    handleSymbol('.');

    std::string subName{ handleIdentifier(Identifiers::Category::SUBROUTINE) };
    Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };

    // If baseName is a variabile → method call on object
    if (kind != Identifiers::VarKind::NONE) 
    {
      int index{ m_symbolTable.indexOf(baseName) };
      std::string type{ m_symbolTable.typeOf(baseName) };
      // push object as arg0
      m_writer.writePush(segmentOf(kind), index);
      nArgs = 1;
      callName = type + "." + subName;
    } 
    else // otherwise it is the name of a class
    {
      callName = baseName + "." + subName;
    }
  } 
  // Case: subroutineName(...) → implicit method on this
  else 
  {
    if (m_currSubKind == Identifiers::SubKind::FUNCTION) 
    {
      throw CompilationError(
          m_fileName, m_tokenizer.tokenLineIdx(),
          "Subroutine " + m_className + "." + baseName + " called as a method from within a function"
      );
    }
    m_writer.writePush(VM::Segment::POINTER, 0); // push this
    nArgs = 1;
    callName = m_className + "." + baseName;
  }

  handleSymbol('(');
  int exprCount = compileExpressionList();
  handleSymbol(')');

  m_writer.writeCall(callName, nArgs + exprCount);
}

void CompilationEngine::compileDo()
{
  handleKeyword(KeyWords::KeyWord::DO);
  
  compileSubroutineCall();
  handleSymbol(';');
  // "do" discards the return value
  m_writer.writePop(VM::Segment::TEMP, 0);
}

void CompilationEngine::compileLet()
{
  handleKeyword(KeyWords::KeyWord::LET);

  // varName
  std::string name{ handleIdentifier(Identifiers::Category::VAR) };
  Identifiers::VarKind kind{ m_symbolTable.kindOf(name) };

  if (kind == Identifiers::VarKind::NONE) 
  {
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                           "Undefined variable '" + name + "' in let statement");
  }

  int index{ m_symbolTable.indexOf(name) };
  VM::Segment baseSeg{ segmentOf(kind) };

  // Case 1: let x[expr] = expr;
  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '[') 
  {
    handleSymbol('[');
    // push base
    m_writer.writePush(baseSeg, index);
    // push expr
    compileExpression();
    handleSymbol(']');
    handleSymbol('=');
    //push RHS expr
    compileExpression();

    m_writer.writePop(VM::Segment::TEMP, 0);    // save RHS
    m_writer.writeArithmetic(VM::Command::ADD); // sum base+offset
    m_writer.writePop(VM::Segment::POINTER, 1); // THAT = base+offset
    m_writer.writePush(VM::Segment::TEMP, 0);   // restore RHS
    m_writer.writePop(VM::Segment::THAT, 0);    // *THAT = RHS

    handleSymbol(';');

    return;
  }
  // Case 2: let x = expr;
  handleSymbol('=');
  compileExpression();
  m_writer.writePop(baseSeg, index);
  handleSymbol(';');
}

void CompilationEngine::compileWhile()
{
  std::string label1{ getNewLabel() };
  std::string label2{ getNewLabel()  };

  handleKeyword(KeyWords::KeyWord::WHILE);

  m_writer.writeLabel(label1);
  handleSymbol('(');
  compileExpression();
  m_writer.writeArithmetic(VM::Command::NOT);
  handleSymbol(')');
  m_writer.writeIf(label2);

  handleSymbol('{');
  compileStatements();
  handleSymbol('}');
  m_writer.writeGoto(label1);

  m_writer.writeLabel(label2);
}

void CompilationEngine::compileReturn()
{
  handleKeyword(KeyWords::KeyWord::RETURN);

  if (m_tokenizer.tokenType() != TokenType::SYMBOL || m_tokenizer.symbol() != ';') 
  {
    if (m_isCurrSubVoid)
    {
      throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), 
                             "A void function must not return a value");
    }
    compileExpression();
  } 
  else 
  {
    // Push 0 as default
    m_writer.writePush(VM::Segment::CONST, 0);
  }

  m_writer.writeReturn();
  handleSymbol(';');
}

void CompilationEngine::compileIf()
{
  std::string label1{ getNewLabel() };
  std::string label2{ getNewLabel() };

  handleKeyword(KeyWords::KeyWord::IF);

  handleSymbol('(');
  compileExpression();
  m_writer.writeArithmetic(VM::Command::NOT);
  handleSymbol(')');
  
  m_writer.writeIf(label1);
  handleSymbol('{');
  compileStatements();
  handleSymbol('}');
  m_writer.writeGoto(label2);

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

    if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',') handleSymbol(',');  
    else break; 
  }

  return exprCount;
}

void CompilationEngine::compileExpression()
{
  compileTerm();

  while (isOperator())
  {
    char op{ handleOperator() };
    compileTerm();

    switch (op)
    {
      case '*': m_writer.writeCall("Math.multiply", 2);              break;
      case '/': m_writer.writeCall("Math.divide", 2);                break;
      default : m_writer.writeArithmetic(VM::command_from_char(op)); break;
    }
  }
}

void CompilationEngine::compileTerm()
{
  // 1) IDENTIFIER: VarName | VarName[expression] | Subroutine call
  if (m_tokenizer.tokenType() == TokenType::IDENTIFIER) 
  {
    std::string baseName{ handleIdentifierOneOf({Identifiers::Category::SUBROUTINE,
                                                 Identifiers::Category::CLASS,
                                                 Identifiers::Category::VAR})
    };                
    // Subroutine call: (className | varName).subroutineName(expressionList) | subroutineName(expressionList)
    if ((m_tokenizer.tokenType() == TokenType::SYMBOL) &&
        ((m_tokenizer.symbol() == '.') || (m_tokenizer.symbol() == '(')))
    {
      compileSubroutineCall(baseName);
    }
    // VarName[expression]
    else if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '[')
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
    }
    // Simple varName → push value
    else
    {
      Identifiers::VarKind kind{ m_symbolTable.kindOf(baseName) };
      if (kind != Identifiers::VarKind::NONE)
      {
        int index { m_symbolTable.indexOf(baseName) };
        m_writer.writePush(segmentOf(kind), index);
      }
      else 
      {
        throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(),
                               "Undefined variable '" + baseName + "'");
      }
    }
  }
  // 2) Keyword constant
  else if ((m_tokenizer.tokenType() == TokenType::KEYWORD) &&
           (m_tokenizer.keyWord() == KeyWords::KeyWord::TRUE  ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::FALSE ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::NULL_ ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::THIS  ))
  {
    
    KeyWords::KeyWord kw{ handleKeywordOneOf({KeyWords::KeyWord::TRUE, KeyWords::KeyWord::FALSE, 
                        KeyWords::KeyWord::NULL_, KeyWords::KeyWord::THIS}) 
    };
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
    default: break; // never
    }
  }
  // 3) Unary op + term
  else if ((m_tokenizer.tokenType() == TokenType::SYMBOL) &&
           (m_tokenizer.symbol() == '-' || m_tokenizer.symbol() == '~'))
  {
    char op{ handleUnaryOperator() };
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
    int val{ handleIntConst() };
    m_writer.writePush(VM::Segment::CONST, val);
  }
  // 6) String constant
  else if (m_tokenizer.tokenType() == TokenType::STRING_CONST)
  {
    std::string str{ handleStringConst() };
    m_writer.writePush(VM::Segment::CONST, static_cast<int>(str.size()));
    m_writer.writeCall("String.new", 1);

    for (const auto c : str) 
    {
      m_writer.writePush(VM::Segment::CONST, static_cast<int>(c));
      m_writer.writeCall("String.appendChar", 2);
    }
  }
  // 7) Error
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
