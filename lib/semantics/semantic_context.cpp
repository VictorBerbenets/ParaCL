#include "semantic_context.hpp"
#include "statement.hpp"

namespace paracl {

ast::statement_block *
SymTable::getDeclScopeFor(const SymbNameType &Name,
                          ast::statement_block *CurrScope) {
  for (; CurrScope; CurrScope = CurrScope->scope()) {
    if (NamesInfo.contains({Name, CurrScope}))
      return CurrScope;
  }
  return nullptr;
}

PCLType *SymTable::getTypeFor(const SymbNameType &Name,
                              ast::statement_block *CurrScope) {
  auto *Decl = getDeclScopeFor(Name, CurrScope);
  if (!Decl)
    return nullptr;

  return NamesInfo.find({Name, Decl})->second.getType();
}

bool SymTable::isDefined(SymTabKey TabKey) {
  return getDeclScopeFor(TabKey.Name, TabKey.CurrScope) != nullptr;
}

} // namespace paracl
