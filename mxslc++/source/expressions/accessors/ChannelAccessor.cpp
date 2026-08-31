//
// Created by jaket on 28/05/2026.
//

#include "expressions/accessors/ChannelAccessor.h"

#include "runtime/variables/ChannelVariable.h"

namespace mxslc::expressions
{
    ChannelAccessor::ChannelAccessor(ExprPtr value_expr, ExprPtr index_expr)
    {
        channel_var_ = std::make_shared<ChannelVariable>(std::move(value_expr), std::move(index_expr));
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
