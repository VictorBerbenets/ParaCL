#pragma once


#if !defined(yyFlexLexerOnce)
#undef yyFlexLexer
#define yyFlexLexer graph_FlexLexer
#include <FlexLexer.h>
#endif

#undef YY_DECL
#define YY_DECL yy::parser::symbol_type yy::scanner::get_next_token()

#include "paracl_grammar.tab.hh"

namespace yy {
class scanner : public yyFlexLexer {
public:
  scanner() {}
  parser::symbol_type get_next_token();
};
} // namespace yy