//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ChannelAccessor.h"

#include "expressions/Expression.h"
#include "runtime/variables/ChannelVariable.h"

namespace mxslc::expressions
{
    ChannelAccessor::ChannelAccessor(const ExprPtr& value_expr, const ExprPtr& index_expr)
    {
        VarPtr value = value_expr->evaluate();
        VarPtr index = index_expr->evaluate();
        channel_var_ = std::make_shared<ChannelVariable>(std::move(value), std::move(index));
    }

    TypePtr ChannelAccessor::type() const
    {
        return channel_var_->type();
    }

    VarPtr ChannelAccessor::evaluate() const
    {
        return channel_var_;
    }
}
