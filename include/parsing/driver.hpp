#pragma once

#include <optional>
#include <string>
#include <utility>

#include "ast.hpp"
#include "error_handler.hpp"
#include "paracl_grammar.tab.hh"
#include "scanner.hpp"

namespace yy {

class driver final {
public:
  driver() : scanner_{}, parser_(scanner_, *this) {}

  void parse();

  void switch_input_stream(std::istream *Is) {
    scanner_.switch_streams(Is, nullptr);
  }

  template <paracl::ast::derived_from NodeType, typename... Args>
  NodeType *make_node(Args &&...args) {
    auto node = ast_.make_node<NodeType>(std::forward<Args>(args)...);
    return node;
  }

  statement_block *make_block();

  root_statement_block *make_root_block();

  void set_ast_root(root_statement_block *root) & noexcept;

  root_statement_block *get_root() const;

  void change_scope(statement_block *new_block) noexcept;

  statement_block *get_current_block() noexcept;

  std::optional<paracl::ErrorHandler> validate() const;

  void evaluate(std::ostream &output = std::cout,
                std::istream &input = std::cin);

  void compile(llvm::StringRef ModuleName, llvm::raw_ostream &Os);

private:
  scanner scanner_;
  parser parser_;
  std::string file_;

  paracl::ast::ast ast_;
};

} // namespace yy
