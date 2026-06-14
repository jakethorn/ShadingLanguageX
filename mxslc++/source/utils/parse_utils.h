//
// Created by jaket on 17/05/2026.
//

#ifndef MXSLC_PARSE_UTILS_H
#define MXSLC_PARSE_UTILS_H

#include "primitive_t.h"
#include "utils/common.h"

FuncPtr parse_function(const string& code);
primitive_t parse_literal(const string& code);

#endif //MXSLC_PARSE_UTILS_H
