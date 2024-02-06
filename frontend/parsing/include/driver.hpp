#pragma once

#include <string>
#include <utility>
#include <optional>
#include <concepts>

#include "ast.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"
#include "interpreter.hpp"
#include "error_handler.hpp"

namespace yy {

class driver final {
 public:
  driver(): scanner_{}, parser_(scanner_, *this) {}

  void parse() {
      parser_.parse();
  }

  void switch_input_stream(std::istream *Is) {
      scanner_.switch_streams(Is, nullptr);
  }

  template <frontend::ast::derived_from NodeType, typename... Args>
  NodeType *make_node(Args&&... args) {
    auto node = ast_.make_node<NodeType>(std::forward<Args>(args)...);
    if (std::same_as<variable, NodeType>) {
      handler_.visit(node);
    }
    return node;
  }

  statement_block *make_block() {
    return ast_.make_block();
  }

  void set_ast_root(statement_block *root) & noexcept {
    ast_.set_root(root);
  }

  void change_scope(statement_block *new_block) noexcept {
    ast_.set_curr_block(new_block);
  }

  statement_block *get_current_block() noexcept {
    return ast_.get_curr_block();
  }

  std::optional<frontend::error_handler> check_for_errors() const {
    if (handler_.empty()) {
      return {};
    }
    return {handler_};
  }

  void evaluate(std::ostream &output = std::cout, std::istream &input = std::cin) const {
    frontend::interpreter runner(input, output);
    runner.run_program(ast_.root_ptr());
  }
 private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  frontend::error_handler handler_;
  frontend::ast::ast ast_;
};

} // <--- namespace yy
