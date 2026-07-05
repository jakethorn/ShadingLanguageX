//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_TOKEN_UTILS_H
#define MXSLC_TOKEN_UTILS_H

#include <string>
#include <vector>

#include "Token.h"

namespace mxslc::token_utils
{
    inline std::string join(const std::vector<Token>& tokens, const std::string& separator = "")
    {
        std::string result;
        for (auto it = tokens.begin(); it != tokens.end(); ++it)
        {
            const Token& token = *it;
            result += token.lexeme();
            if (it < tokens.end()-1)
                result += separator;
        }
        return result;
    }
}

#endif //MXSLC_TOKEN_UTILS_H
