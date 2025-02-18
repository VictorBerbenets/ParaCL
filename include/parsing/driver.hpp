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

namespace yy {

class driver final {
public:
  driver() : scanner_{}, parser_(scanner_, *this) {}

  void parse() { parser_.parse(); }

  void switch_input_stream(std::istream *Is) {
    scanner_.switch_streams(Is, nullptr);
  }

  template <paracl::ast::derived_from NodeType, typename... Args>
  NodeType *make_node(Args &&...args) {
    auto node = ast_.make_node<NodeType>(std::forward<Args>(args)...);
    if (std::same_as<variable, NodeType>) {
      handler_.visit(node);
    }
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

  std::optional<paracl::error_handler> check_for_errors() const {
    if (handler_.empty()) {
      return {};
    }
    return {handler_};
  }

  void evaluate(std::ostream &output = std::cout,
                std::istream &input = std::cin) const {
    paracl::interpreter runner(input, output);
    runner.run_program(ast_.root_ptr());
  }

  void compile(llvm::StringRef Output) const {
    paracl::CodeGenVisitor GenVis(Output);
    GenVis.generateIRCode(ast_.root_ptr());
  }

private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  paracl::error_handler handler_;
  paracl::ast::ast ast_;
};

} // namespace yy
