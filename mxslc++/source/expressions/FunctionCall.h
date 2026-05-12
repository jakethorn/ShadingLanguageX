//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONCALL_H
#define MXSLC_FUNCTIONCALL_H

#include "Expression.h"
#include "utils/common.h"
#include "runtime/ArgumentList.h"

class FunctionCall : public Expression
{
public:
    FunctionCall(string name, ArgumentList args);
    FunctionCall(string name, ArgumentList args, Token token);
    FunctionCall(string name, TypePtr template_type, ArgumentList args);
    FunctionCall(string name, TypePtr template_type, ArgumentList args, Token token);
    FunctionCall(string name, TypePtr template_type, ArgumentList args, AttributeList attrs);
    FunctionCall(string name, TypePtr template_type, ArgumentList args, AttributeList attrs, Token token);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

    virtual vector<FuncPtr> get_matching_functions(const vector<TypePtr>& return_types) const;
    virtual FuncPtr get_matching_function(const vector<TypePtr>& return_types) const;

    string name_;
    TypePtr template_type_;
    ArgumentList args_;

    FuncPtr func_ = nullptr;

private:
    bool arguments_are_initialized();
    size_t try_init_arguments(const vector<FuncPtr>& funcs);
    void evaluate_arguments() const;
    void update_out_arguments() const;

    size_t initialized_arg_count_ = 0;
};

#endif //MXSLC_FUNCTIONCALL_H
