#pragma once


#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer graph_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL yy::parser::symbol_type yy::scanner::get_next_token()

#include "paracl_grammar.tab.hh"
#include "location.hh"

namespace yy {
class scanner : public yyFlexLexer {
  position m_pos;

  auto symbol_length() const { return yyleng; }
public:
  scanner() {}
  parser::symbol_type get_next_token();

  location update_location() {
    auto old_pos = m_pos;
    auto new_pos = (m_pos += symbol_length());
    return location{old_pos, new_pos};
  }
};
} // namespace yy