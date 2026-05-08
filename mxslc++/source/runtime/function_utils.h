//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_FUNCTION_UTILS_H
#define MXSLC_FUNCTION_UTILS_H

#include "ArgumentList.h"
#include "utils/common.h"

bool is_matching_function(
    const FuncPtr& func,
    const string& name
);

bool is_matching_function(
    const FuncPtr& func,
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
);

vector<FuncPtr> get_matching_functions(
    const vector<FuncPtr>& funcs,
    const string& name
);

vector<FuncPtr> get_matching_functions(
    const vector<FuncPtr>& funcs,
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
);

FuncPtr get_matching_function(
    const vector<FuncPtr>& funcs,
    const vector<TypePtr>& return_types,
    const string& name,
    const TypePtr& template_type,
    const ArgumentList& args
);

#endif //MXSLC_FUNCTION_UTILS_H
