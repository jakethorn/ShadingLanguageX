//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_FUNCTIONQUERY_H
#define MXSLC_FUNCTIONQUERY_H

#include "ArgumentList.h"
#include "utils/common.h"

class FunctionQuery
{
public:
    FunctionQuery() = default;
    explicit FunctionQuery(const string& name);
    FunctionQuery(const string& name, const TypePtr& template_type);
    FunctionQuery(const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args);
    FunctionQuery(const TypePtr& class_type, const vector<TypePtr>& return_types, const string& name, const TypePtr& template_type, const ArgumentList& args);

    // non-owning pointers
    const TypePtr* class_type = nullptr;
    const vector<TypePtr>* return_types = nullptr;
    const string* name = nullptr;
    const TypePtr* template_type = nullptr;
    const ArgumentList* args = nullptr;

    bool is_match(const FuncPtr& func) const;
    bool has_match(const vector<FuncPtr>& funcs) const;
    FuncPtr get_match(const vector<FuncPtr>& funcs, bool throw_on_fail) const;
    vector<FuncPtr> get_matches(const vector<FuncPtr>& funcs) const;

private:
    bool class_type_matches(const FuncPtr& func) const;
    bool return_type_matches(const FuncPtr& func) const;
    bool name_matches(const FuncPtr& func) const;
    bool template_type_matches(const FuncPtr& func) const;
    bool arguments_match(const FuncPtr& func) const;

    vector<FuncPtr> get_default_functions(const vector<FuncPtr>& funcs) const;
};

#endif //MXSLC_FUNCTIONQUERY_H
