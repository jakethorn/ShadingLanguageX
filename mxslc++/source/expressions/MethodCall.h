//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_METHODCALL_H
#define MXSLC_METHODCALL_H

#include "FunctionCall.h"

class MethodCall final : public FunctionCall
{
public:
    MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args);
    MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, Token token);
    MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, AttributeList attrs);
    MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, ArgumentList args, AttributeList attrs, Token token);

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    VarPtr evaluate_impl() const override;

    vector<FuncPtr> get_matching_functions(const vector<TypePtr>& return_types) const override;
    FuncPtr get_matching_function(const vector<TypePtr>& return_types) const override;

private:
    void set_this() const;

    ExprPtr instance_expr_;
    VarPtr instance_;
};

#endif //MXSLC_METHODCALL_H
