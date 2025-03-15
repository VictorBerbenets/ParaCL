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

SymTabKey SymTable::getDeclKeyFor(const SymbNameType &Name,
                                  ast::statement_block *CurrScope) {
  auto *DeclScope = getDeclScopeFor(Name, CurrScope);
  return {Name, DeclScope};
}

SymTabKey SymTable::getDeclKeyFor(const SymTabKey &Key) {
  auto *DeclScope = getDeclScopeFor(Key.Name, Key.CurrScope);
  return {Key.Name, DeclScope};
}

PCLType *SymTable::getTypeFor(const SymbNameType &Name,
                              ast::statement_block *CurrScope) {
  auto *Decl = getDeclScopeFor(Name, CurrScope);
  if (!Decl)
    return nullptr;

  return NamesInfo.find({Name, Decl})->second;
}

PCLType *SymTable::getTypeFor(const SymTabKey &Key) {
  return getTypeFor(Key.Name, Key.CurrScope);
}

ast::statement_block *SymTable::getDeclScopeFor(const SymTabKey &Key) {
  return getDeclScopeFor(Key.Name, Key.CurrScope);
}

bool SymTable::isDefined(SymTabKey TabKey) {
  return getDeclScopeFor(TabKey.Name, TabKey.CurrScope) != nullptr;
}

bool SymTable::containsKeyWithType(PCLType *Ty) const {
  return llvm::any_of(llvm::make_second_range(NamesInfo),
                      [Ty](auto &&InfoTy) { return Ty == InfoTy; });
}

} // namespace paracl
