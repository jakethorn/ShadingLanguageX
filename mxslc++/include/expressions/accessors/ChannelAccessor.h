//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_CHANNELACCESSOR_H
#define MXSLC_CHANNELACCESSOR_H

#include "expressions/accessors/Accessor.h"
#include "common.h"

namespace mxslc::expressions
{
    class ChannelAccessor final : public Accessor
    {
    public:
        ChannelAccessor(const ExprPtr& value_expr, const ExprPtr& index_expr);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        VarPtr channel_var_;
    };
}

#endif //MXSLC_CHANNELACCESSOR_H
