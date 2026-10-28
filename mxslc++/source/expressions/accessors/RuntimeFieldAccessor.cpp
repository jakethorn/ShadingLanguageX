//
// Created by jaket on 27/10/2026.
//

#include "expressions/accessors/RuntimeFieldAccessor.h"

#include "errors/CompileError.h"
#include "expressions/interface.h"
#include "expressions/SwitchExpression.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

namespace mxslc::expressions
{
    RuntimeFieldAccessor::RuntimeFieldAccessor(ExprPtr value_expr, VarPtr index)
        : value_expr_{std::move(value_expr)}, index_{std::move(index)}
    {
        const TypePtr value_type = value_expr_->type();
        for (size_t i = 1; i < value_type->field_count(); ++i)
        {
            if (not value_type->field_type(i)->equals(value_type->field_type(0), /*field_names*/false))
                throw CompileError{"Fields of type '" + value_type->to_string() + "' must all be the same type to be accessed using an index that is not known at compile-time"};
        }
    }

    TypePtr RuntimeFieldAccessor::type() const
    {
        return value_expr_->type()->field_type(0);
    }

    VarPtr RuntimeFieldAccessor::evaluate() const
    {
        const VarPtr value = value_expr_->evaluate();

        vector<SwitchCase> cases;
        cases.reserve(value->child_count());
        for (const VarPtr& field : value->children())
            cases.emplace_back(as_expression(field));

        const SwitchExpressionPtr switch_expr = create_expression<SwitchExpression>(as_expression(index_), std::move(cases));
        switch_expr->init();
        return switch_expr->evaluate();
    }
}
