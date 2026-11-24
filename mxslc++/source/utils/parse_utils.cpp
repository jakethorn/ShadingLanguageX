//
// Created by jaket on 17/05/2026.
//

#include "utils/parse_utils.h"
#include "scan.h"
#include "errors/CompileError.h"

namespace mxslc::parse_utils
{
    Primitive parse_literal(const string& code)
    {
        const vector<Token> tokens = scan_string(code);
        if (tokens.size() == 1)
            return tokens[0].literal();
        if (tokens.size() == 2)
        {
            const Primitive value = tokens[1].literal();
            if (tokens[0] == '-')
                return -value;
        }

        throw CompileError{"Invalid literal: " + code};
    }
}
