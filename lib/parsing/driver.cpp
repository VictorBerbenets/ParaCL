
#include <llvm/Support/CommandLine.h>

#include "codegen_visitor.hpp"
#include "driver.hpp"
#include "interpreter.hpp"
#include "option_category.hpp"

namespace cl = llvm::cl;

cl::opt<std::string> DumpCfg("dump-cfg",
                             cl::desc("dump control flow graph in a dot file"),
                             cl::value_desc("dot file name"), cl::Optional,
                             cl::init("paracl-dump-cfg.dot"),
                             cl::cat(paracl::ParaCLCategory));

namespace yy {

void driver::parse() { parser_.parse(); }

statement_block *driver::make_block() { return ast_.make_block(); }

root_statement_block *driver::make_root_block() {
  return ast_.make_root_block();
}

void driver::set_ast_root(root_statement_block *root) & noexcept {
  ast_.set_root(root);
}

root_statement_block *driver::get_root() const { return ast_.root_ptr(); }

void driver::change_scope(statement_block *new_block) noexcept {
  ast_.set_curr_block(new_block);
}

statement_block *driver::get_current_block() noexcept {
  return ast_.get_curr_block();
}

std::optional<paracl::ErrorHandler> driver::validate() const {
  paracl::ErrorHandler Handler;
  Handler.run_program(ast_.root_ptr());
  if (Handler.empty()) {
    return {};
  }
  return {std::move(Handler)};
}

void driver::evaluate(std::ostream &output, std::istream &input) {
  paracl::Interpreter runner(input, output);
  runner.run_program(ast_.root_ptr());
}

void driver::compile(llvm::StringRef ModuleName, llvm::raw_ostream &Os) {
  paracl::CodeGenVisitor CodeGenVis(ModuleName);
  CodeGenVis.generateIRCode(ast_.root_ptr(), Os);
  if (DumpCfg.getNumOccurrences() > 0)
    CodeGenVis.dumpInDotFormat(DumpCfg);
}

} // namespace yy
