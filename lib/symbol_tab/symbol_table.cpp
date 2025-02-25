#include "symbol_table.hpp"
#include "statement.hpp"

namespace paracl {
  
  ast::statement_block *SymTable::getDeclScopeFor(const llvm::StringRef &Name, ast::statement_block *CurrScope) {
    for (; CurrScope; CurrScope = CurrScope->scope())
      if (NamesInfo.contains({Name, CurrScope}))
        return true;
  }
  
  PCLType *SymTable::getTypeFor(const llvm::StringRef &Name, ast::statement_block *CurrScope) {
    assert(NamesInfo.contains({Name, CurrScope})); 
    return NamesInfo[{Name, CurrScope}].Ty.get();
  }
  
  bool SymTable::isDefined(SymTabKey TabKey) const { 
    for (auto *CurrScope = TabKey.CurrScope; CurrScope; CurrScope = CurrScope->scope())
      if (NamesInfo.contains({TabKey.Name, CurrScope}))
        return true;
    return false;
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
