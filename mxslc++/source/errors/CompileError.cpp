//
// Created by jaket on 14/11/2025.
//

#include "errors/CompileError.h"
#include "Token.h"
#include "utils/string_utils.h"

namespace mxslc
{
    using namespace string_utils;

    string CompileError::format(const Token& debug_info, const string& message)
    {
        if (debug_info.line() == 0)
        {
            return message;
        }

        if (debug_info.filename().empty())
        {
            return "line " + str(debug_info.line()) + ": " + message;
        }

        return debug_info.filename() + ", line " + str(debug_info.line()) + ": " + message;
    }
}
