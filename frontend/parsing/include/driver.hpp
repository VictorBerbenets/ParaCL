#pragma once

#include <string>
#include <utility>

#include "ast.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"
#include "print_visitor.hpp"

namespace yy {

class driver {
 public:
  driver(): scanner_{}, parser_(scanner_, *this) {}

  void parse() {
      parser_.parse();
  }

  void switchInputStream(std::istream *Is) {
      scanner_.switch_streams(Is, nullptr);
  }

  template <frontend::ast::derived_from NodeType, typename... Args>
  NodeType* make_node(Args... args) {
    return ast_.make_node<NodeType>(std::forward<Args>(args)...);
  }

  void set_ast_root(i_node *root) & noexcept {
    ast_.set_root(root);
  }

  void print_ast(const std::string &file_name) {
    frontend::print_visitor p_visitor(file_name);
    p_visitor.visit(static_cast<statement_block*>(ast_.root_ptr()));
  }

 private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  frontend::ast::ast ast_;
};

} // <--- namespace yy
