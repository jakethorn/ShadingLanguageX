//
// Created by jaket on 07/07/2026.
//

#include "utils/token_utils.h"
#include "Token.h"

namespace mxslc::token_utils
{
    string join_tokens(const vector<Token>& tokens, const string& separator)
    {
        string result;
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
