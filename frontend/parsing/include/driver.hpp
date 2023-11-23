#pragma once

#include <string>

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "paracl_grammar.tab.hh"

#define YY_DECL \
    yy::parser::symbol_type yylex (yy::Driver* drv)
YY_DECL;

namespace yy {

class Driver {
    FlexLexer *plex_;
    yy::location location_;
    std::string file_;
public:
    Driver(FlexLexer *plex) : plex_(plex) {}

    // parser::symbol_type yylex() {
    //     auto tt = static_cast<yy::parser::token::token_kind_type>(plex_->yylex());
    //     switch(tt) {
    //       case yy::parser::token::TOK_NUMBER:
    //           yylval-> = std::stoi(plex_->YYText());
    //           break;
    //       case yy::parser::token::TOK_VAR:
    //           yylval->value = plex_->YYText();
    //           break;
    //       default: { /* do nothing */}
    //     }
    //     return tt;
    // }

    bool parse(const std::string& f) {
        file_ = f;
        parser parser(this);
        location_.initialize (&f);

        bool res = parser.parse();
        return !res;
    }

    yy::location location() const {
        return location_;
    }
};

} // <--- namespace yy