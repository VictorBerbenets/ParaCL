#pragma once

#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer graph_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL yy::parser::symbol_type yy::scanner::get_token()

#include "paracl_grammar.tab.hh"
#include "location.hh"

namespace yy {
class scanner final: public yyFlexLexer {
 public:
  scanner() {}
  parser::symbol_type get_token();

  location update_location() {
    if (curr_line_ != yylineno) {
      curr_line_ = yylineno;
    }
    auto old_pos = pos_;
    auto new_pos = (pos_ += yyleng);
    std::cout << yylineno << std::endl;
    location loc_val {old_pos, new_pos};
    loc_val.lines(curr_line_);
    return loc_val;
  }
 private:
  position pos_;
  int curr_line_ = 1;

};

} // <--- namespace yy
