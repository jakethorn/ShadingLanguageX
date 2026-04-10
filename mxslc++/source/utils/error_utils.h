//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_ERROR_UTILS_H
#define MXSLC_ERROR_UTILS_H

#include "common.h"

class ArgumentList;
class Token;

string missing_overload_error(
    const FuncPtr& func,
    const vector<TypeInfoPtr>& return_types,
    const Token& name,
    const TypeInfoPtr& template_type,
    const ArgumentList& args
);

string missing_overload_error(
    const vector<FuncPtr>& funcs,
    const vector<TypeInfoPtr>& return_types,
    const Token& name,
    const TypeInfoPtr& template_type,
    const ArgumentList& args
);

string ambiguous_overload_error(const vector<FuncPtr>& funcs);

#endif //MXSLC_ERROR_UTILS_H
