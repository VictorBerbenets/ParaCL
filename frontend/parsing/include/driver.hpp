#pragma once

#include <string>
#include <utility>

#include "ast.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

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

  void set_ast_root(ast_node* root) & noexcept {
    ast_.set_root(root);
  }

 private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  frontend::ast::ast ast_;
};

} // <--- namespace yy
