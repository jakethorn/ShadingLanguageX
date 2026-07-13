//
// Created by jaket on 17/05/2026.
//

#ifndef MXSLC_PARSE_UTILS_H
#define MXSLC_PARSE_UTILS_H

#include "common.h"
#include "primitive/Primitive.h"

namespace mxslc::parse_utils
{
    FuncPtr parse_function(const string& code);
    Primitive parse_literal(const string& code);
}

#endif //MXSLC_PARSE_UTILS_H
