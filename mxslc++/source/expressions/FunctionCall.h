//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_FUNCTIONCALL_H
#define FENNEC_FUNCTIONCALL_H

#include "Expression.h"
#include "utils/common.h"
#include "utils/template_utils.h"
#include "Token.h"
#include "runtime/ArgumentList.h"

class Function;

class FunctionCall final : public Expression
{
public:
    FunctionCall(const Runtime& runtime, Token name, ArgumentList args)
        : FunctionCall{runtime, std::move(name), std::nullopt, std::move(args)} { }

    FunctionCall(const Runtime& runtime, Token name, optional<Token> template_type, ArgumentList args)
        : Expression{runtime, std::move(name)},
        template_type_{std::move(template_type)},
        args_{std::move(args)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    void init_impl(const vector<TypeInfoPtr>& types) override;
    [[nodiscard]] TypeInfoPtr type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    TypeInfoPtr template_type() const;
    size_t try_init_arguments(const vector<FuncPtr>& funcs);
    void evaluate_arguments() const;
    [[nodiscard]] ValuePtr evaluate_return() const;

    optional<Token> template_type_;
    ArgumentList args_;

    size_t initialised_arg_count_ = 0;
    FuncPtr func_ = nullptr;
};

#endif //FENNEC_FUNCTIONCALL_H
