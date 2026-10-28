//
// Created by jaket on 11/01/2026.
//

#include "expressions/IndexingOperator.h"

#include "errors/CompileError.h"
#include "expressions/interface.h"
#include "expressions/accessors/ChannelAccessor.h"
#include "expressions/accessors/FieldAccessor.h"
#include "expressions/accessors/RuntimeFieldAccessor.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "serialize/Serializer.h"

namespace mxslc::expressions
{
    IndexingOperator::IndexingOperator(ExprPtr value_expr, ExprPtr index_expr)
        : Expression{index_expr->token()}, value_expr_{std::move(value_expr)}, index_expr_{std::move(index_expr)}
    {

    }

    ExprPtr IndexingOperator::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<IndexingOperator>(
            value_expr_->monomorphize(template_type),
            index_expr_->monomorphize(template_type)
        );
    }

    void IndexingOperator::init_subexpressions(const vector<TypePtr>& types)
    {
        if (not value_expr_->try_init(Type::tuples_of(types)))
            value_expr_->init();

        index_expr_->init(Type::Int);
    }

    void IndexingOperator::init_impl(const vector<TypePtr>& types)
    {
        if (value_expr_->type()->is_primitive())
        {
            accessor_ = create_accessor<ChannelAccessor>(value_expr_, index_expr_);
        }
        else
        {
            serializer().begin_comptime();
            const VarPtr index = index_expr_->evaluate();
            serializer().end_comptime();

            if (index->is_compile_time())
                accessor_ = create_accessor<FieldAccessor>(value_expr_, index->compile_time_value<int>());
            else
                accessor_ = create_accessor<RuntimeFieldAccessor>(value_expr_, index);
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

    void IndexingOperator::assign_impl(const VarPtr& value) const
    {
        if (is_a<RuntimeFieldAccessor>(accessor_))
            throw CompileError{"Cannot assign to a field using an index that is not known at compile-time"};

        Expression::assign_impl(value);
    }

    string IndexingOperator::to_string() const
    {
        return value_expr_->to_string() + "[" + index_expr_->to_string() + "]";
    }
}
