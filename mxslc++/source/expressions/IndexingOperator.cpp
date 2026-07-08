//
// Created by jaket on 11/01/2026.
//

#include "expressions/IndexingOperator.h"

#include "expressions/interface.h"
#include "expressions/accessors/ComponentAccessor.h"
#include "expressions/accessors/FieldAccessor.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    IndexingOperator::IndexingOperator(ExprPtr value_expr, ExprPtr index_expr)
        : Expression{index_expr->token()}, value_expr_{std::move(value_expr)}, index_expr_{std::move(index_expr)}
    {

    }

    ExprPtr IndexingOperator::instantiate_template_types(const TypePtr& template_type) const
    {
        return create_expression<IndexingOperator>(
            value_expr_->instantiate_template_types(template_type),
            index_expr_->instantiate_template_types(template_type)
        );
    }

    void IndexingOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        value_expr_->init();
        index_expr_->init(Type::Int);
    }

    void IndexingOperator::init_impl(const vector<TypePtr>& types)
    {
        if (value_expr_->type()->is_primitive())
        {
            accessor_ = create_accessor<ComponentAccessor>(value_expr_, index_expr_);
        }
        else
        {
            VarPtr var = value_expr_->evaluate();
            const int index = index_expr_->evaluate()->value_as<int>();
            accessor_ = create_accessor<FieldAccessor>(std::move(var), index);
        }
    }

    TypePtr IndexingOperator::type_impl() const
    {
        return accessor_->type();
    }

    VarPtr IndexingOperator::evaluate_impl() const
    {
        return accessor_->evaluate();
    }
}
