//
// Created by jaket on 14/06/2026.
//

#include "TypeOfOperator.h"

#include "runtime/Field.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

TypeOfOperator::TypeOfOperator(ExprPtr expr) : expr_{std::move(expr)}
{

}

TypeOfOperator::TypeOfOperator(ExprPtr expr, Token token) : Expression{std::move(token)}, expr_{std::move(expr)}
{

}

ExprPtr TypeOfOperator::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<TypeOfOperator>(std::move(expr), token_);
}

void TypeOfOperator::init_subexpressions(const vector<TypePtr>& types)
{
    expr_->init();
}

void TypeOfOperator::init_impl(const vector<TypePtr>& types)
{
    Field name_field{TokenType::Const, Type::String, "name"};
    Field str_field{TokenType::Const, Type::String, "str"};
    type_ = std::make_unique<Type>(vector{std::move(name_field), std::move(str_field)});
}

TypePtr TypeOfOperator::type_impl() const
{
    return type_;
}

VarPtr TypeOfOperator::evaluate_impl() const
{
    const TypePtr type = expr_->evaluate()->type();
    return Variable::create(TokenType::Const, type_, {type->name(), type->str()});
}
