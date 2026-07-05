//
// Created by jaket on 03/07/2026.
//

#include "Macro.h"

#include "scan.h"
#include "utils/common.h"

namespace mxslc
{
    Macro::Macro(string name) : name_{std::move(name)} { }
    Macro::Macro(string name, vector<Token> body) : name_{std::move(name)}, body_{std::move(body)} { }
    Macro::Macro(string name, const string& body) : Macro{std::move(name), scan_string(body)} { }
}
