//
// Created by jaket on 04/01/2026.
//

#include "ReturnStatement.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"

ReturnStatement::ReturnStatement(const Runtime& runtime, Type type, ExprPtr expr) : Statement{runtime}, type_{std::move(type)}, expr_{std::move(expr)}
{

}

StmtPtr ReturnStatement::instantiate_templated_types(const Type& template_type) const
{
    Type type = type_.instantiate_template_type(template_type);
    ExprPtr expr = expr_->instantiate_templated_types(template_type);
    return std::make_unique<ReturnStatement>(runtime_, type, std::move(expr));
}

void ReturnStatement::init()
{
    expr_->init(type_);
}

void ReturnStatement::execute()
{
    const ValuePtr value = expr_->evaluate();
    runtime_.serializer().write_return(value);
}
