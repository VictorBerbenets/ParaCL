#pragma once

#include <string>

#include "paracl_grammar.tab.hh"
#include <FlexLexer.h>

namespace yy {

class Driver {
    FlexLexer *plex_;

public:
    Driver(FlexLexer *plex) : plex_(plex) {}

    parser::token_type yylex(parser::semantic_type *yylval) {
        parser::token_type tt = static_cast<parser::token_type>(plex_->yylex());
        switch(tt) {
          case yy::parser::token_type::NUMBER :
              yylval->as<int>() = std::stoi(plex_->YYText());
              break;
          case yy::parser::token_type::VAR :
              yylval->as<std::string>() = plex_->YYText();
              break;
          default: { /* do nothing */}
        }

        return tt;
    }

    bool parse() {
        parser parser(this);
        bool res = parser.parse();
        return !res;
    }
};

} // namespace yy
