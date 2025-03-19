#pragma once

#include <llvm/IR/Module.h>

namespace paracl {
namespace codegen {

void dumpInDotFormat(const llvm::Module &Mod, llvm::StringRef FileNameToDump);

} // namespace codegen
} // namespace paracl
