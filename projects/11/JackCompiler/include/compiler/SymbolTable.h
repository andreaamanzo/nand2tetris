#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <map>
#include "utils/Identifiers.h"

class SymbolTable
{
private:
  std::map<std::string, Identifiers::Symbol> m_classScopeTable{};
  std::map<std::string, Identifiers::Symbol> m_subroutineScopeTable{};
  int m_staticCount{ 0 };
  int m_fieldCount { 0 };
  int m_argCount   { 0 };
  int m_varCount   { 0 };

  const Identifiers::Symbol* findSymbol(const std::string& name) const noexcept;

  static constexpr bool isClassScope(Identifiers::VarKind k) noexcept 
  {
    return k == Identifiers::VarKind::FIELD || k == Identifiers::VarKind::STATIC;
  }

public:
  SymbolTable() = default;

  void startSubroutine() noexcept;
  void define(const std::string& name, const std::string& type, Identifiers::VarKind kind);
  int varCount(Identifiers::VarKind kind) const noexcept;

  Identifiers::VarKind kindOf(const std::string& name)  const noexcept;
  std::string typeOf(const std::string& name)  const;
  int indexOf(const std::string& name) const;
};

#endif