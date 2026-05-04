//
// Created by jaket on 25/03/2026.
//

#include "DotExpression.h"

#include "Identifier.h"
#include "dot_accessors/FieldAccessor.h"
#include "dot_accessors/PortAccessor.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

DotExpression::DotExpression(ExprPtr expr, Token property) : Expression{std::move(property)}, expr_{std::move(expr)}
{

}

ExprPtr DotExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<DotExpression>(std::move(expr), token_);
}

void DotExpression::init_subexpressions(const vector<TypePtr>& types)
{
    expr_->init();
}

void DotExpression::init_impl(const vector<TypePtr>& types)
{
    VarPtr var = expr_->evaluate();
    if (var->has_value())
    {
        accessor_ = std::make_shared<PortAccessor>(std::move(var), token_.lexeme());
    }
    else
    {
        accessor_= std::make_shared<FieldAccessor>(std::move(var), token_.lexeme());
    }
}

TypePtr DotExpression::type_impl() const
{
    return accessor_->type();
}

VarPtr DotExpression::evaluate_impl() const
{
    return accessor_->evaluate();
}
