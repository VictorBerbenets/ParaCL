#pragma once

#include <string>

#include "ast.hpp"
#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

namespace yy {

class Driver {
  public:

    Driver(): scanner_{}, parser_(scanner_, *this) {}

    void parse() {
        parser_.parse();
    }

    void switchInputStream(std::istream* Is) {
        scanner_.switch_streams(Is, nullptr);
    }

  private:
    scanner scanner_;
    parser parser_;
    std::string file_;

    frontend::ast::ast ast_;
};

} // <--- namespace yy
