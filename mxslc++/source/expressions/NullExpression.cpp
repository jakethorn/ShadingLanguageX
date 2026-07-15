//
// Created by jaket on 31/12/2025.
//

#include "expressions/NullExpression.h"

#include "runtime/interface.h"
#include "errors/CompileError.h"
#include "expressions/interface.h"
#include "serialize/values/interface.h"
#include "serialize/values/NullValue.h"

namespace mxslc::expressions
{
    ExprPtr NullExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<NullExpression>(token_);
    }

    void NullExpression::init_impl(const vector<TypePtr>& types)
    {
        if (types.size() != 1)
            throw CompileError{"Ambiguous null expression"};
        type_ = types[0];
    }

    TypePtr NullExpression::type_impl() const
    {
        return type_;
    }

    VarPtr NullExpression::evaluate_impl() const
    {
        ValuePtr value = create_value<NullValue>(type_);
        return create_variable(std::move(value));
    }

    string NullExpression::to_string() const
    {
        return "null";
    }
}
