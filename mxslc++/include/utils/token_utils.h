//
// Created by jaket on 05/07/2026.
//

#ifndef MXSLC_TOKEN_UTILS_H
#define MXSLC_TOKEN_UTILS_H

#include "common.h"

namespace mxslc
{
    class Token;
}

namespace mxslc::token_utils
{
    string join(const vector<Token>& tokens, const string& separator = "");
}

#endif //MXSLC_TOKEN_UTILS_H
