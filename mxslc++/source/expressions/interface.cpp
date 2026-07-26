//
// Created by jaket on 26/07/2026.
//

#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"

namespace mxslc::expressions
{
    RuntimeExpressionPtr as_expression(Primitive value)
    {
        return create_expression<RuntimeExpression>(std::move(value));
    }

    RuntimeExpressionPtr as_expression(ValuePtr value)
    {
        return create_expression<RuntimeExpression>(std::move(value));
    }

    RuntimeExpressionPtr as_expression(VarPtr value)
    {
        return create_expression<RuntimeExpression>(std::move(value));
    }
}
