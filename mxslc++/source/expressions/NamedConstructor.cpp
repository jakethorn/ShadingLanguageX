//
// Created by jaket on 05/05/2026.
//

#include "NamedConstructor.h"

#include "FunctionCall.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"

NamedConstructor::NamedConstructor(Token name, ArgumentList args)
    : Expression{std::move(name)}, args_{std::move(args)}
{

}

ExprPtr NamedConstructor::instantiate_template_types(const TypePtr& template_type) const
{
    Token token = token_.instantiate_template_types(template_type);
    ArgumentList args = args_.instantiate_template_types(template_type);
    return std::make_unique<NamedConstructor>(std::move(token), std::move(args));
}

void NamedConstructor::init_impl(const vector<TypePtr>& types)
{
    const TypePtr type = scope().get_type(token_.lexeme());
    func_call_ = std::make_shared<FunctionCall>("__" + type->name() + "__", std::move(args_));
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
