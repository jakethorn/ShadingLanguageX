//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_FUNCTIONCALL_H
#define FENNEC_FUNCTIONCALL_H

#include <cassert>

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

    [[nodiscard]] ExprPtr instantiate_templated_types(const Type& template_type) const override;

    [[nodiscard]] const string& name() const { return token_.lexeme(); }
    [[nodiscard]] bool has_template_type() const { return template_type_.has_value(); }
    [[nodiscard]] const Type& template_type() const { return template_type_.value(); }
    [[nodiscard]] const ArgumentList& arguments() const { return args_; }
    [[nodiscard]] const Function& function() const
    {
        assert(is_initialized_);
        return *func_;
    }

protected:
    void init_child_expressions(const vector<Type>& types) override;
    void init_impl(const vector<Type>& types) override;
    [[nodiscard]] const Type& type_impl() const override;
    [[nodiscard]] ValuePtr evaluate_impl() const override;

private:
    void try_init_arguments(const vector<const Function*>& funcs);

    optional<Type> template_type_;
    ArgumentList args_;

    size_t initialised_args_count_ = 0;
    const Function* func_ = nullptr;
};

#endif //FENNEC_FUNCTIONCALL_H
