//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONCALL_H
#define MXSLC_FUNCTIONCALL_H

#include "expressions/Expression.h"
#include "common.h"
#include "runtime/ArgumentList.h"

namespace mxslc::expressions
{
    class FunctionCall : public Expression
    {
    public:
        explicit FunctionCall(string name);
        FunctionCall(string name, optional<ArgumentList> args);
        FunctionCall(string name, optional<ArgumentList> args, Token token);
        FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args);
        FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, Token token);
        FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs);
        FunctionCall(string name, TypePtr template_type, optional<ArgumentList> args, AttributeList attrs, Token token);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

        void evaluate_arguments() const;
        VarPtr inline_invoke() const;
        void update_out_arguments() const;

        string name_;
        TypePtr template_type_;
        ArgumentList args_;
        bool is_argumentless_{false};

        FuncPtr func_;

    private:
        bool is_method_call() const { return method_call_ != nullptr;}

        ExprPtr method_call_;
    };
}

#endif //MXSLC_FUNCTIONCALL_H
