//
// Created by jaket on 14/11/2025.
//

#include "CompileError.h"
#include "Token.h"

string CompileError::format(const Token& debug_info, const string& message)
{
    if (debug_info.line() == 0)
    {
        return message;
    }

    if (debug_info.filename().empty())
    {
        return "line " + std::to_string(debug_info.line()) + ": " + message;
    }

    return debug_info.filename() + ", line " + std::to_string(debug_info.line()) + ": " + message;
}
