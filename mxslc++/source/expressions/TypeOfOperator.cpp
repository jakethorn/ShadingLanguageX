//
// Created by jaket on 14/06/2026.
//

#include "TypeOfOperator.h"

#include "Identifier.h"
#include "runtime/Field.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "utils/instantiate_template_types_utils.h"

TypeOfOperator::TypeOfOperator(ExprPtr expr) : TypeOfOperator{std::move(expr), nullptr, Token{}}
{

}

TypeOfOperator::TypeOfOperator(ExprPtr expr, Token token) : TypeOfOperator{std::move(expr), nullptr, std::move(token)}
{

}

TypeOfOperator::TypeOfOperator(ExprPtr expr, TypePtr template_type, Token token) : Expression{std::move(token)}, expr_{std::move(expr)}, template_type_{std::move(template_type)}
{

}

ExprPtr TypeOfOperator::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<TypeOfOperator>(std::move(expr), template_type, token_);
}

void TypeOfOperator::init_subexpressions(const vector<TypePtr>& types)
{
    if (const shared_ptr<Identifier> identifier = std::dynamic_pointer_cast<Identifier>(expr_))
    {
        string type_name = identifier->name();
        if (template_type_)
            type_name = ::instantiate_template_types(type_name, template_type_);

        if (scope().has_type(type_name))
        {
            expr_type_ = scope().get_type(type_name);
            return;
        }
    }

    expr_->init();
    expr_type_ = expr_->type();
}

void TypeOfOperator::init_impl(const vector<TypePtr>& types)
{
    Field name_field{TokenType::Const, Type::String, "name"};
    Field str_field{TokenType::Const, Type::String, "str"};
    op_type_ = std::make_unique<Type>(vector{std::move(name_field), std::move(str_field)});
}

TypePtr TypeOfOperator::type_impl() const
{
    return op_type_;
}

VarPtr TypeOfOperator::evaluate_impl() const
{
    return Variable::create(TokenType::Const, op_type_, {expr_type_->name(), expr_type_->full_str()});
}
