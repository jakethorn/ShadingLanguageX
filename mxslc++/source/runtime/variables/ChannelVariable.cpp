//
// Created by jaket on 13/07/2026.
//

#include "runtime/variables/ChannelVariable.h"
#include "runtime/ArgumentList.h"
#include "expressions/interface.h"
#include "expressions/FunctionCall.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/utils/invoke.h"

namespace mxslc::runtime
{
    ChannelVariable::ChannelVariable(ExprPtr value_expr, ExprPtr index_expr)
        : Variable{TokenType::Mutable, get_type(value_expr, index_expr)}, value_expr_{std::move(value_expr)}, index_expr_{std::move(index_expr)}
    {

    }

    ValuePtr ChannelVariable::value_impl() const
    {
        if (channel_value_ == nullptr)
            channel_value_ = runtime_utils::invoke_function("__get__", ArgumentList{value_expr_, index_expr_})->value();
        return channel_value_;
    }

    void ChannelVariable::copy_value_impl(ValuePtr value)
    {
        runtime_utils::invoke_function("__set__", ArgumentList{value_expr_, index_expr_, as_expression(std::move(value))});
    }

    void ChannelVariable::set_node_name(const string& name) const
    {

    }

    TypePtr ChannelVariable::get_type(const ExprPtr& value_expr, const ExprPtr& index_expr)
    {
        const FunctionCallPtr getter = create_expression<FunctionCall>("__get__", ArgumentList{value_expr, index_expr});
        getter->init();
        return getter->type();
    }
}
