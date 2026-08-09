//
// Created by jaket on 06/05/2026.
//

#ifndef MXSLC_METHODCALL_H
#define MXSLC_METHODCALL_H

#include "expressions/FunctionCall.h"

namespace mxslc::expressions
{
    class MethodCall final : public FunctionCall
    {
    public:
        MethodCall(ExprPtr instance_expr, string method_name, optional<ArgumentList> args);
        MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args);
        MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, Token token);
        MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs);
        MethodCall(ExprPtr instance_expr, string method_name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs, Token token);

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        VarPtr evaluate_impl() const override;

    private:
        VarPtr copy_instance_to_scope() const;
        void update_instance(const VarPtr& local_copy) const;

        ExprPtr instance_expr_;
        VarPtr instance_;
    };
}

#endif //MXSLC_METHODCALL_H
