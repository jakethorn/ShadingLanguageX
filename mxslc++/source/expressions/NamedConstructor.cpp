//
// Created by jaket on 05/05/2026.
//

#include "expressions/NamedConstructor.h"

#include "expressions/FunctionCall.h"
#include "runtime/RuntimeUtils.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "utils/instantiate_template_types_utils.h"

NamedConstructor::NamedConstructor(string name, ArgumentList args)
    : name_{std::move(name)}, args_{std::move(args)}
{

}

NamedConstructor::NamedConstructor(string name, ArgumentList args, Token token)
    : Expression{std::move(token)}, name_{std::move(name)}, args_{std::move(args)}
{

}

ExprPtr NamedConstructor::instantiate_template_types(const TypePtr& template_type) const
{
    string name = ::instantiate_template_types(name_, template_type);
    ArgumentList args = args_.instantiate_template_types(template_type);
    return std::make_unique<NamedConstructor>(std::move(name), std::move(args), token_);
}

void NamedConstructor::init_impl(const vector<TypePtr>& types)
{
    const TypePtr type = scope().get_type(name_);
    func_call_ = RuntimeUtils::function_call("__" + type->name() + "__", std::move(args_), token_);
    func_call_->init(type);
}

TypePtr NamedConstructor::type_impl() const
{
    return func_call_->type();
}

VarPtr NamedConstructor::evaluate_impl() const
{
    return func_call_->evaluate();
}
