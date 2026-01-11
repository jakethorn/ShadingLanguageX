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

    FunctionCall(const Runtime& runtime, Token name, optional<Type> template_type, ArgumentList args)
        : Expression{runtime, std::move(name)},
        template_type_{std::move(template_type)},
        args_{std::move(args)} { }

    [[nodiscard]] ExprPtr instantiate_template_types(const Type& template_type) const override;

protected:
    void init_subexpressions(const vector<Type>& types) override;
    void init_impl(const vector<Type>& types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    void try_init_arguments(const vector<const Function*>& funcs);
    void evaluate_arguments() const;
    [[nodiscard]] ValuePtr evaluate_return() const;

    optional<Type> template_type_;
    ArgumentList args_;

    size_t initialised_arg_count_ = 0;
    const Function* func_ = nullptr;
};

#endif //FENNEC_FUNCTIONCALL_H
