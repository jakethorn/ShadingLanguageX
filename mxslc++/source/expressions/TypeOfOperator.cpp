//
// Created by jaket on 14/06/2026.
//

#include "expressions/TypeOfOperator.h"

#include "expressions/Identifier.h"
#include "expressions/interface.h"
#include "runtime/Field.h"
#include "runtime/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::expressions
{
    TypeOfOperator::TypeOfOperator(ExprPtr expr) : TypeOfOperator{std::move(expr), nullptr, Token{}}
    {

    }

    TypeOfOperator::TypeOfOperator(ExprPtr expr, Token token) : TypeOfOperator{std::move(expr), nullptr, std::move(token)}
    {

    }

    TypeOfOperator::TypeOfOperator(ExprPtr expr, TypePtr template_type, Token token) : Expression{std::move(token)}, expr_{std::move(expr)}, template_type_{std::move(template_type)}
    {

    }

    ExprPtr TypeOfOperator::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = expr_->monomorphize(template_type);
        return create_expression<TypeOfOperator>(std::move(expr), template_type, token_);
    }

    void TypeOfOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        if (const IdentifierPtr identifier = cast_expression<Identifier>(expr_))
        {
            string type_name = identifier->name();
            if (template_type_)
                type_name = runtime_utils::monomorphize(type_name, template_type_);

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
        vector<Field> fields {
            {TokenType::Const, Type::String, "name"},
            {TokenType::Const, Type::String, "str"}
        };

        const TypePtr type = create_type(std::move(fields));
        op_type_ = scope().resolve_type(type);
    }

    TypePtr TypeOfOperator::type_impl() const
    {
        return op_type_;
    }

    VarPtr TypeOfOperator::evaluate_impl() const
    {
        return create_variable(TokenType::Const, op_type_, {expr_type_->name(), expr_type_->full_str()});
    }

    string TypeOfOperator::to_string() const
    {
        return "typeof(" + expr_->to_string() + ")";
    }
}
