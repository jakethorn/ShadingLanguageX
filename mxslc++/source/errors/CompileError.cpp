//
// Created by jaket on 14/11/2025.
//

#include "errors/CompileError.h"
#include "Token.h"

namespace mxslc
{
    string CompileError::to_string(const Token& debug_info)
    {
        if (debug_info.line() == 0)
            return "";
        if (debug_info.filename().empty())
            return "line " + std::to_string(debug_info.line()) + ": ";
        return debug_info.filename() + ", line " + std::to_string(debug_info.line()) + ": ";
    }
}
