#include <string>
#include <stdexcept>
#include "utils/Identifiers.h"
#include "compiler/SymbolTable.h"

const Identifiers::Symbol* SymbolTable::findSymbol(const std::string& name) const noexcept 
{
  if (auto it = m_subroutineScopeTable.find(name); it != m_subroutineScopeTable.end())
    return &it->second;

  if (auto it = m_classScopeTable.find(name); it != m_classScopeTable.end())
    return &it->second;

  return nullptr;
}

void SymbolTable::startSubroutine() noexcept
{
  m_subroutineScopeTable.clear();
  m_argCount = 0;
  m_varCount = 0;
}

void SymbolTable::define(const std::string& name, const std::string& type, Identifiers::VarKind kind)
{
  auto& table{ isClassScope(kind) ? m_classScopeTable : m_subroutineScopeTable };

  if (table.contains(name)) 
  {
    throw std::logic_error("SymbolTable::define(): Symbol '" + name + "' already defined in this scope");
  }

  int index = { varCount(kind) };
  switch (kind) 
  {
  case Identifiers::VarKind::STATIC: {++m_staticCount; break;}
  case Identifiers::VarKind::FIELD:  {++m_fieldCount;  break;}
  case Identifiers::VarKind::ARG:    {++m_argCount;    break;}
  case Identifiers::VarKind::VAR:    {++m_varCount;    break;}
  default: throw std::invalid_argument("SymbolTable::define(): Invalid kind parameter (Identifiers::VarKind::NONE)");
  }

  Identifiers::Symbol newSymbol{ name, type, kind, index };
  table[name] = std::move(newSymbol);
}

int SymbolTable::varCount(Identifiers::VarKind kind) const noexcept
{
  switch (kind) 
  {
  case Identifiers::VarKind::STATIC: return m_staticCount;
  case Identifiers::VarKind::FIELD:  return m_fieldCount;
  case Identifiers::VarKind::ARG:    return m_argCount;
  case Identifiers::VarKind::VAR:    return m_varCount;
  default:                           return 0; // unreachable
  } 
}

Identifiers::VarKind SymbolTable::kindOf(const std::string& name) const noexcept
{
  if (auto s = findSymbol(name))
    return s->kind;

  return Identifiers::VarKind::NONE;
}

std::string SymbolTable::typeOf(const std::string& name) const
{
  if (auto s = findSymbol(name))
    return s->type;

  throw std::logic_error("SymbolTable::typeOf(): Undefined identifier '" + name + "'");
}

int SymbolTable::indexOf(const std::string& name) const 
{
  if (auto s = findSymbol(name))
    return s->index;

  throw std::logic_error("SymbolTable::indexOf(): Undefined identifier '" + name + "'");
}