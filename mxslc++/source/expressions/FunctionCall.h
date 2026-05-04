//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONCALL_H
#define MXSLC_FUNCTIONCALL_H

#include "Expression.h"
#include "utils/common.h"
#include "Token.h"
#include "runtime/ArgumentList.h"

class FunctionCall final : public Expression
{
public:
    FunctionCall(Token name, TypePtr template_type, ArgumentList args)
        : Expression{std::move(name)}, template_type_{std::move(template_type)}, args_{std::move(args)} { }

    FunctionCall(Token name, ArgumentList args)
        : FunctionCall{std::move(name), nullptr, std::move(args)} { }

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypePtr>& types) override;
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool arguments_are_initialized();
    size_t try_init_arguments(const vector<FuncPtr>& funcs);
    void evaluate_arguments() const;
    void update_out_arguments() const;

    TypePtr template_type_;
    ArgumentList args_;

    size_t initialized_arg_count_ = 0;
    FuncPtr func_ = nullptr;
};

#endif //MXSLC_FUNCTIONCALL_H
