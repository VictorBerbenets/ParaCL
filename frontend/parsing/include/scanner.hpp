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
class scanner : public yyFlexLexer {
 public:
  scanner() {}
  parser::symbol_type get_token();

  location update_location() {
    auto old_pos = pos_;
    auto new_pos = (pos_ += yyleng);
    return {old_pos, new_pos};
  }
 private:
  position pos_;

};

} // <--- namespace yy
