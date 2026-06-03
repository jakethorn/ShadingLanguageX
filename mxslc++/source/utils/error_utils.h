//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_ERROR_UTILS_H
#define MXSLC_ERROR_UTILS_H

#include "common.h"
#include "CompileError.h"

CompileError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs);

#endif //MXSLC_ERROR_UTILS_H
