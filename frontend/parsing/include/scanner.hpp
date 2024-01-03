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

#include <iomanip>

namespace yy {
class scanner final: public yyFlexLexer {
 public:
  scanner() {}
  parser::symbol_type get_token();

  location update_location() {
    auto start_column = curr_column_;
    if (curr_line_ != yylineno) {
      curr_line_   = yylineno;
      curr_column_ = start_column = 0;
    } else {
      start_column += 1;
      curr_column_ += yyleng;
    }

    position old_pos {YY_NULLPTR, curr_line_, start_column};
    position new_pos {YY_NULLPTR, curr_line_, curr_column_ + 1};
    location loc_val {old_pos, new_pos};

    return loc_val;
  }

 private:
  int curr_line_   = 1;
  int curr_column_ = 0;
};

} // <--- namespace yy
