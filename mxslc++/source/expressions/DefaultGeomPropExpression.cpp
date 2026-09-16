//
// Created by jaket on 15/09/2026.
//

#include "expressions/DefaultGeomPropExpression.h"

#include "expressions/interface.h"
#include "runtime/interface.h"
#include "serialize/values/DefaultGeomPropValue.h"
#include "serialize/values/interface.h"
#include "serialize/values/StringValue.h"

namespace mxslc::expressions
{
    DefaultGeomPropExpression::DefaultGeomPropExpression(string name, Token token) : Expression{std::move(token)}, name_{std::move(name)}
    {

    }

    ExprPtr DefaultGeomPropExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<DefaultGeomPropExpression>(name_, token_);
    }

    TypePtr DefaultGeomPropExpression::type_impl() const
    {
        return create_value<DefaultGeomPropValue>(name_)->type();
    }

    VarPtr DefaultGeomPropExpression::evaluate_impl() const
    {
        return create_variable(
            create_value<DefaultGeomPropValue>(name_)
        );
    }

    string DefaultGeomPropExpression::to_string() const
    {
        return "geomprop(" + name_ + ")";
    }
}
