//
// Created by jaket on 03/07/2026.
//

#include "preprocess/Macro.h"

#include "scan.h"
#include "common.h"
#include "preprocess/preprocess.h"
#include "utils/container_utils.h"

namespace mxslc::preprocess
{
    using container_utils::contains;

    Macro::Macro(string name) : Macro{std::move(name), vector<Token>{}} { }
    Macro::Macro(string name, const string& body) : Macro{std::move(name), scan_string(body)} { }
    Macro::Macro(string name, vector<Token> body) : name_{std::move(name)}, body_{std::move(body)}
    {
        validate();
    }

    void Macro::validate() const
    {
        if (Token{name_} != TokenType::Identifier or contains(Preprocessor::DIRECTIVES, name_))
            throw CompileError{"Invalid macro name: " + name_};
    }
}
