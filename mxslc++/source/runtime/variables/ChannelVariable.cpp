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
    ChannelVariable::ChannelVariable(VarPtr value, VarPtr index)
        : Variable{TokenType::Mutable, get_type(value, index)}, value_{std::move(value)}, index_{std::move(index)}
    {

    }

    ValuePtr ChannelVariable::value_impl() const
    {
        if (channel_value_ == nullptr)
            channel_value_ = runtime_utils::invoke_function("__get__", ArgumentList{value_, index_})->value();
        return channel_value_;
    }

    void ChannelVariable::copy_value_impl(ValuePtr value)
    {
        runtime_utils::invoke_function("__set__", ArgumentList{value_, index_, as_expression(std::move(value))});
    }

    void ChannelVariable::set_node_name(const string& name) const
    {

    }

    TypePtr ChannelVariable::get_type(const VarPtr& value, const VarPtr& index)
    {
        const FunctionCallPtr getter = create_expression<FunctionCall>("__get__", ArgumentList{value, index});
        getter->init();
        return getter->type();
    }
}
