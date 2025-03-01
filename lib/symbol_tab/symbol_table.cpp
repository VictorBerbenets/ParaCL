#include "symbol_table.hpp"
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
#ifdef DEBUG
  std::cout << "SYMBOLE TABLE DUMP\n";
  for (auto &[Key, _] : NamesInfo)
    std::cout << "KeyName = " << std::string(Key.Name)
              << "; Scope = " << std::hex << Key.CurrScope << std::dec << '\n';
  std::cout << "IS NULL PTR = " << (Decl == nullptr) << '\n';
  std::cout << "Name = " << std::string(Name) << "; Scope = " << std::hex
            << CurrScope << '\n';
#endif

  // assert(NamesInfo.contains({Name, Decl}));
  return NamesInfo.find({Name, Decl})->second.getType();
}

bool SymTable::isDefined(SymTabKey TabKey) {
  return getDeclScopeFor(TabKey.Name, TabKey.CurrScope) != nullptr;
}
#if 0
bool symbol_table::has(const std::string &var_name) const {
  return names_.find(var_name) != names_.end();
}

void symbol_table::add(const std::string &var_name, int value) {
  names_.insert({var_name, value});
}

symbol_table::value_type &symbol_table::operator[](const std::string &name) {
  return names_[name];
}
#endif

} // namespace paracl
