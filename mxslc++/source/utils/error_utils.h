//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_ERROR_UTILS_H
#define MXSLC_ERROR_UTILS_H

#include "AmbiguousFunctionError.h"
#include "common.h"
#include "CompileError.h"

AmbiguousFunctionError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs);
AmbiguousFunctionError ambiguous_function_error(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors);

#endif //MXSLC_ERROR_UTILS_H
