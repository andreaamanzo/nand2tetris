#include <fstream>
#include <string>
#include <algorithm>
#include <initializer_list>
#include "IOFiles.h"
#include "CompilationEngine.h"
#include "JackTokenizer.h"
#include "TokenType.h"
#include "KeyWords.h"
#include "CompilationError.h"

namespace 
{
  std::string xmlEscape(char c) {
    switch (c) {
      case '<': return "&lt;";
      case '>': return "&gt;";
      case '&': return "&amp;";
      case '"': return "&quot;";
      case '\'': return "&apos;";
      default:  return std::string(1, c);
    }
  }
}

CompilationEngine::CompilationEngine(InputFile& inputFile, std::ofstream& outputFile)
  : m_fileName(inputFile.fileName + ".jack")
  , m_tokenizer(inputFile)
  , m_outputFile(outputFile)
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

// ------- EXPECT HELPERS -------

void CompilationEngine::expectIdentifier(Identifiers::Identifier identifier)
{
  if (m_tokenizer.tokenType() != TokenType::IDENTIFIER) 
  {
    throw CompilationError(
      m_fileName, m_tokenizer.tokenLineIdx(),
      "Expected " + Identifiers::to_string(identifier) + " identifier"
    );
  }
}

void CompilationEngine::expectIdentifierOneOf(std::initializer_list<Identifiers::Identifier> allowed)
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

// ------- HANDLERS (expect + write + advance) -------

void CompilationEngine::handleSymbol(char symbol)
{
  expectSymbol(symbol);
  m_outputFile << "<symbol> " << xmlEscape(m_tokenizer.symbol()) << " </symbol>\n";
  advanceOrError();
}

void CompilationEngine::handleKeyword(KeyWords::KeyWord keyWord)
{
  expectKeyword(keyWord);
  m_outputFile << "<keyword> " << KeyWords::to_string(m_tokenizer.keyWord()) << " </keyword>\n";
  advanceOrError();
}

void CompilationEngine::handleKeywordOneOf(std::initializer_list<KeyWords::KeyWord> allowed)
{
  expectKeywordOneOf(allowed);
  m_outputFile << "<keyword> " << KeyWords::to_string(m_tokenizer.keyWord()) << " </keyword>\n";
  advanceOrError();
}

void CompilationEngine::handleIdentifier(Identifiers::Identifier identifier)
{
  expectIdentifier(identifier);
  m_outputFile << "<identifier> " << m_tokenizer.identifier() << " </identifier>\n";
  advanceOrError();
}

void CompilationEngine::handleIdentifierOneOf(std::initializer_list<Identifiers::Identifier> allowed)
{
  expectIdentifierOneOf(allowed);
  m_outputFile << "<identifier> " << m_tokenizer.identifier() << " </identifier>\n";
  advanceOrError();
}

void CompilationEngine::handleType(bool voidOption /*= false*/)
{
  expectType(voidOption);
  if (m_tokenizer.tokenType() == TokenType::KEYWORD) {
    m_outputFile << "<keyword> " << KeyWords::to_string(m_tokenizer.keyWord()) << " </keyword>\n";
  } else {
    m_outputFile << "<identifier> " << m_tokenizer.identifier() << " </identifier>\n";
  }
  advanceOrError();
}

// ------- PARSER -------

void CompilationEngine::compileClass()
{
  m_outputFile << "<class>\n";

  handleKeyword(KeyWords::KeyWord::CLASS);
  handleIdentifier(Identifiers::Identifier::CLASS);
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
  m_outputFile << "<symbol> " << xmlEscape(m_tokenizer.symbol()) << " </symbol>\n";
  // No advance

  m_outputFile << "</class>";
}

void CompilationEngine::compileClassVarDec()
{
  m_outputFile << "<classVarDec>\n";

  handleKeywordOneOf({KeyWords::KeyWord::FIELD, KeyWords::KeyWord::STATIC});
  handleType();
  handleIdentifier(Identifiers::Identifier::VAR);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    handleIdentifier(Identifiers::Identifier::VAR);
  }

  handleSymbol(';');

  m_outputFile << "</classVarDec>\n";
}

void CompilationEngine::compileSubroutine()
{
  m_outputFile << "<subroutineDec>\n";

  handleKeywordOneOf({KeyWords::KeyWord::CONSTRUCTOR, KeyWords::KeyWord::FUNCTION, KeyWords::KeyWord::METHOD});
  handleType(true); // allows 'void'
  handleIdentifier(Identifiers::Identifier::SUBROUTINE);
  handleSymbol('(');

  compileParameterList();

  handleSymbol(')');

  m_outputFile << "<subroutineBody>\n";

  handleSymbol('{');

  while (m_tokenizer.tokenType() == TokenType::KEYWORD &&
         m_tokenizer.keyWord() == KeyWords::KeyWord::VAR)
  {
    compileVarDec();
  }

  compileStatements();

  handleSymbol('}');

  m_outputFile << "</subroutineBody>\n";
  m_outputFile << "</subroutineDec>\n";
}

void CompilationEngine::compileParameterList()
{
  m_outputFile << "<parameterList>\n";

  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ')') {
    m_outputFile << "</parameterList>\n";
    return;
  }

  handleType();
  handleIdentifier(Identifiers::Identifier::VAR);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    handleType();
    handleIdentifier(Identifiers::Identifier::VAR);
  }

  m_outputFile << "</parameterList>\n";
}

void CompilationEngine::compileVarDec()
{
  m_outputFile << "<varDec>\n";

  handleKeyword(KeyWords::KeyWord::VAR);
  handleType();
  handleIdentifier(Identifiers::Identifier::VAR);

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    handleIdentifier(Identifiers::Identifier::VAR);
  }

  handleSymbol(';');

  m_outputFile << "</varDec>\n";
}

void CompilationEngine::compileStatements()
{
  m_outputFile << "<statements>\n";

  if (m_tokenizer.tokenType() != TokenType::KEYWORD && 
     !(m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '}'))
  {
    std::string msg = "Expected statement keyword ('let', 'if', 'while', 'do', 'return')";
    throw CompilationError(m_fileName, m_tokenizer.tokenLineIdx(), msg);
  }

  while (m_tokenizer.tokenType() == TokenType::KEYWORD) {
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

  m_outputFile << "</statements>\n";
}

void CompilationEngine::compileDo()
{
  m_outputFile << "<doStatement>\n";

  handleKeyword(KeyWords::KeyWord::DO);
  handleIdentifierOneOf({Identifiers::Identifier::SUBROUTINE, 
                         Identifiers::Identifier::CLASS, 
                         Identifiers::Identifier::VAR});
  
  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '.')
  {
    handleSymbol('.');
    handleIdentifier(Identifiers::Identifier::SUBROUTINE);
  }

  handleSymbol('(');

  compileExpressionList();

  handleSymbol(')');
  handleSymbol(';');

  m_outputFile << "</doStatement>\n";
}

void CompilationEngine::compileLet()
{
  m_outputFile << "<letStatement>\n";

  handleKeyword(KeyWords::KeyWord::LET);
  handleIdentifier(Identifiers::Identifier::VAR);

  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == '[')
  {
    handleSymbol('[');
    
    compileExpression();

    handleSymbol(']');
  }

  handleSymbol('=');

  compileExpression();

  handleSymbol(';');

  m_outputFile << "</letStatement>\n";
}

void CompilationEngine::compileWhile()
{
  m_outputFile << "<whileStatement>\n";

  handleKeyword(KeyWords::KeyWord::WHILE);
  handleSymbol('(');

  compileExpression();

  handleSymbol(')');
  handleSymbol('{');

  compileStatements();

  handleSymbol('}');

  m_outputFile << "</whileStatement>\n";
}

void CompilationEngine::compileReturn()
{
  m_outputFile << "<returnStatement>\n";

  handleKeyword(KeyWords::KeyWord::RETURN);

  if (!(m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ';'))
  {
    compileExpression();
  }

  handleSymbol(';');

  m_outputFile << "</returnStatement>\n";
}

void CompilationEngine::compileIf()
{
  m_outputFile << "<ifStatement>\n";

  handleKeyword(KeyWords::KeyWord::IF);
  handleSymbol('(');

  compileExpression();

  handleSymbol(')');
  handleSymbol('{');

  compileStatements();

  handleSymbol('}');

  if (m_tokenizer.tokenType() == TokenType::KEYWORD && m_tokenizer.keyWord() == KeyWords::KeyWord::ELSE)
  {
    handleKeyword(KeyWords::KeyWord::ELSE);
    handleSymbol('{');

    compileStatements();

    handleSymbol('}');
  }

  m_outputFile << "</ifStatement>\n";
}

void CompilationEngine::compileExpressionList()
{
  m_outputFile << "<expressionList>\n";

  if (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ')') {
    m_outputFile << "</expressionList>\n";
    return;
  }

  compileExpression();

  while (m_tokenizer.tokenType() == TokenType::SYMBOL && m_tokenizer.symbol() == ',')
  {
    handleSymbol(',');
    
    compileExpression();
  }

  m_outputFile << "</expressionList>\n";
}

void CompilationEngine::compileExpression()
{
  m_outputFile << "<expression>\n";

  compileTerm();

  //TODO (op term)*

  m_outputFile << "</expression>\n";
}

void CompilationEngine::compileTerm()
{
  m_outputFile << "<term>\n";

  if (m_tokenizer.tokenType() == TokenType::IDENTIFIER) 
  {
    handleIdentifier(Identifiers::Identifier::VAR);
  }
  else if ((m_tokenizer.tokenType() == TokenType::KEYWORD) &&
           (m_tokenizer.keyWord() == KeyWords::KeyWord::TRUE  ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::FALSE ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::NULL_ ||
            m_tokenizer.keyWord() == KeyWords::KeyWord::THIS  ))
  {
    handleKeywordOneOf({KeyWords::KeyWord::TRUE, KeyWords::KeyWord::FALSE, KeyWords::KeyWord::NULL_, KeyWords::KeyWord::THIS});
  }

  //TODO complete

  m_outputFile << "</term>\n";
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
