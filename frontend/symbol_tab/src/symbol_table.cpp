#include "symbol_table.hpp"

namespace frontend {

bool symbol_table::has(const std::string &var_name) const {
    return names_.find(var_name) != names_.end();
}

} // <--- namespace frontend