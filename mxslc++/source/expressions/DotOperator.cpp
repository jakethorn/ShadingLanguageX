//
// Created by jaket on 25/03/2026.
//

#include "expressions/DotOperator.h"

#include "expressions/interface.h"
#include "expressions/accessors/FieldAccessor.h"
#include "expressions/accessors/PortAccessor.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    DotOperator::DotOperator(ExprPtr expr, Token property) : Expression{std::move(property)}, expr_{std::move(expr)}
    {

    }

    ExprPtr DotOperator::monomorphize(const TypePtr& template_type) const
    {
        ExprPtr expr = expr_->monomorphize(template_type);
        return create_expression<DotOperator>(std::move(expr), token_);
    }

    void DotOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        expr_->init();
    }

    void DotOperator::init_impl(const vector<TypePtr>& types)
    {
        VarPtr var = expr_->evaluate();
        if (var->has_value())
        {
            accessor_ = create_accessor<PortAccessor>(std::move(var), token_.lexeme());
        }
        else
        {
            accessor_ = create_accessor<FieldAccessor>(types, std::move(var), token_.lexeme());
        }
    }

    TypePtr DotOperator::type_impl() const
    {
        return accessor_->type();
    }

    VarPtr DotOperator::evaluate_impl() const
    {
        return accessor_->evaluate();
    }

    string DotOperator::to_string() const
    {
        return expr_->to_string() + "." + token_.lexeme();
    }
}
