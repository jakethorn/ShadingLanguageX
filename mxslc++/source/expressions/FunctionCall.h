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
        : FunctionCall{runtime, std::move(name), nullptr, std::move(args)} { }

    FunctionCall(const Runtime& runtime, Token name, TypeInfoPtr template_type, ArgumentList args)
        : Expression{runtime, std::move(name)}, template_type_{std::move(template_type)}, args_{std::move(args)} { }

    ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const override;

protected:
    void init_subexpressions(const vector<TypeInfoPtr>& types) override;
    void init_impl(const vector<TypeInfoPtr>& types) override;
    TypeInfoPtr type_impl() const override;
    ValuePtr evaluate_impl() const override;

private:
    const string& name() const { return token_.lexeme(); }
    bool arguments_are_initialized();
    size_t try_init_arguments(const vector<FuncPtr>& funcs);
    unordered_map<string, ValuePtr> evaluate_arguments() const;
    void add_arguments_to_scope(const unordered_map<string, ValuePtr>& args) const;
    ValuePtr evaluate_return() const;
    void set_out_arguments() const;

    TypeInfoPtr template_type_;
    ArgumentList args_;

    size_t initialized_arg_count_ = 0;
    FuncPtr func_ = nullptr;
};

#endif //FENNEC_FUNCTIONCALL_H
