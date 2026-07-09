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

    protected:
        void init_subexpressions(const vector<TypePtr>& types) override;
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

        virtual vector<FuncPtr> get_matching_functions(const Scope& scope, const vector<TypePtr>& return_types) const;
        virtual FuncPtr get_matching_function(const Scope& scope, const vector<TypePtr>& return_types) const;

        void evaluate_arguments() const;
        VarPtr inline_invoke() const;
        void update_out_arguments() const;

        string name_;
        TypePtr template_type_;
        ArgumentList args_;
        bool is_argumentless_{false};

        FuncPtr func_;

    private:
        bool arguments_are_initialized();
        void init_arguments(const Scope& scope, const vector<TypePtr>& return_types);
        size_t try_init_arguments(const vector<FuncPtr>& funcs);
        size_t try_init_arguments(const FuncPtr& func);

        size_t initialized_arg_count_{0};
        vector<string> underlying_errors_;
        Scope* func_scope_{nullptr};

        ExprPtr method_call_;
    };
}

#endif //MXSLC_FUNCTIONCALL_H
