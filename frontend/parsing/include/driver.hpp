#pragma once

#include <string>


#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

namespace yy {

class Driver {
public:
    scanner Scan;
    parser Parser;

    yy::location location_;
    std::string file_;

    Driver(): Scan{}, Parser(Scan, *this) {}

    void parse() {
        Parser.parse();
    }

    yy::location location() const {
        return location_;
    }

    void switchInputStream(std::istream* Is) { Scan.switch_streams(Is, nullptr); }
};

} // <--- namespace yy