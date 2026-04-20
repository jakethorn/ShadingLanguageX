//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONCALL2_H
#define MXSLC_FUNCTIONCALL2_H

#include "Expression.h"
#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "runtime/ArgumentList.h"

class Function;

class FunctionCall2 final : public Expression
{
public:
    FunctionCall2(Token name, ArgumentList args)
        : FunctionCall2{std::move(name), nullptr, std::move(args)} { }

    FunctionCall2(Token name, TypeInfoPtr template_type, ArgumentList args)
        : Expression{std::move(name)}, template_type_{std::move(template_type)}, args_{std::move(args)} { }

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    void init_impl(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    VarPtr2 evaluate_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool arguments_are_initialized();
    size_t try_init_arguments(const vector<FuncPtr2>& funcs);
    void add_arguments_to_inline_scope() const;

    TypeInfoPtr template_type_;
    ArgumentList args_;

    size_t initialized_arg_count_ = 0;
    FuncPtr2 func_ = nullptr;
};

#endif //MXSLC_FUNCTIONCALL2_H
