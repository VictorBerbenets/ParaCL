#pragma once

#include <concepts>
#include <optional>
#include <string>
#include <utility>

#include "ast.hpp"
#include "codegen_visitor.hpp"
#include "error_handler.hpp"
#include "interpreter.hpp"
#include "paracl_grammar.tab.hh"
#include "scanner.hpp"
#include "semantic_context.hpp"

namespace yy {

class driver final {
public:
  driver()
      : scanner_{}, parser_(scanner_, *this), Handler(SymTab, ValManager) {}

  void parse() { parser_.parse(); }

  void switch_input_stream(std::istream *Is) {
    scanner_.switch_streams(Is, nullptr);
  }

  template <paracl::ast::derived_from NodeType, typename... Args>
  NodeType *make_node(Args &&...args) {
    auto node = ast_.make_node<NodeType>(std::forward<Args>(args)...);
#if 0
    if (std::same_as<variable, NodeType>) {
      Handler.visit(node);
    }
#endif
    return node;
  }

  statement_block *make_block() { return ast_.make_block(); }

  root_statement_block *make_root_block() { return ast_.make_root_block(); }

  void set_ast_root(root_statement_block *root) & noexcept {
    ast_.set_root(root);
  }

  root_statement_block *get_root() const { return ast_.root_ptr(); }

  void change_scope(statement_block *new_block) noexcept {
    ast_.set_curr_block(new_block);
  }

  statement_block *get_current_block() noexcept {
    return ast_.get_curr_block();
  }

  std::optional<paracl::ErrorHandler> check_for_errors() const {
    if (Handler.empty()) {
      return {};
    }
    return {Handler};
  }

  paracl::SymTable &getSymTab() { return SymTab; }
  paracl::ValueManager &getValManager() { return ValManager; }

  void evaluate(std::ostream &output = std::cout,
                std::istream &input = std::cin) {
 //   Handler.run_program(ast_.root_ptr());
    paracl::interpreter runner(SymTab, ValManager, input, output);
    runner.run_program(ast_.root_ptr());
  }

  void compile(llvm::StringRef ModuleName, llvm::raw_ostream &Os) {
//    Handler.run_program(ast_.root_ptr());
    paracl::CodeGenVisitor CodeGenVis(SymTab, ValManager, ModuleName);
    CodeGenVis.generateIRCode(ast_.root_ptr(), Os);
  }

private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  paracl::ast::ast ast_;
  paracl::SymTable SymTab;
  paracl::ValueManager ValManager;
  paracl::ErrorHandler Handler;
};

} // namespace yy
