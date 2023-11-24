#pragma once

#include <string>


#include "scanner.hpp"
#include "paracl_grammar.tab.hh"

namespace my_yy {

class Driver {
public:
    scanner Scan;
    parser Parser;

    std::string file_;

    Driver(): Scan{}, Parser(Scan, *this) {}

    void parse() {
        Parser.parse();
    }

    void switchInputStream(std::istream* Is) { Scan.switch_streams(Is, nullptr); }
};

} // <--- namespace yy