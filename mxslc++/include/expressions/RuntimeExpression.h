//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_RUNTIMEEXPRESSION_H
#define MXSLC_RUNTIMEEXPRESSION_H

#include "expressions/Expression.h"

namespace mxslc::expressions
{
    class RuntimeExpression final : public Expression
    {
    public:
        explicit RuntimeExpression(Primitive value);
        explicit RuntimeExpression(ValuePtr value);
        explicit RuntimeExpression(VarPtr value);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        VarPtr value_;
    };
}

#endif //MXSLC_RUNTIMEEXPRESSION_H
