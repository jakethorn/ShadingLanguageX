//
// Created by jaket on 26/07/2026.
//

#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/variables/Variable.h"
#include "expressions/Literal.h"
#include "expressions/UnnamedConstructor.h"

namespace mxslc::expressions
{
    ExprPtr as_expression(Primitive value)
    {
        return create_expression<RuntimeExpression>(std::move(value));
    }

    ExprPtr as_expression(ValuePtr value)
    {
        return create_expression<RuntimeExpression>(std::move(value));
    }

    ExprPtr as_expression(VarPtr value)
    {
        if (value->is_external())
        {
            if (value->has_children())
            {
                vector<ExprPtr> children;
                children.reserve(value->child_count());
                for (const VarPtr& child : value->children())
                    children.push_back(as_expression(child));
                return create_expression<UnnamedConstructor>(std::move(children));
            }
            else if (value->is_compile_time())
            {
                return create_expression<Literal>(value->compile_time_value());
            }
        }

        return create_expression<RuntimeExpression>(std::move(value));
    }
}
