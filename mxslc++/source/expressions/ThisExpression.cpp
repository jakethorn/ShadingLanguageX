//
// Created by jaket on 06/05/2026.
//

#include "expressions/ThisExpression.h"

#include "expressions/interface.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    ExprPtr ThisExpression::instantiate_template_types(const TypePtr& template_type) const
    {
        return create_expression<ThisExpression>(token_);
    }

    void ThisExpression::init_impl(const vector<TypePtr>& types)
    {
        this_ = scope().get_variable("this");
    }

    TypePtr ThisExpression::type_impl() const
    {
        return this_->type();
    }

    VarPtr ThisExpression::evaluate_impl() const
    {
        return this_;
    }
}
