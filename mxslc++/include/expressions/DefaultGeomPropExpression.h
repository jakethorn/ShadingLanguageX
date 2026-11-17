//
// Created by jaket on 15/09/2026.
//

#ifndef MXSLC_DEFAULTGEOMPROPEXPRESSION_H
#define MXSLC_DEFAULTGEOMPROPEXPRESSION_H

#include "expressions/Expression.h"
#include "serialize/values/DefaultGeomPropValue.h"

namespace mxslc::expressions
{
    class DefaultGeomPropExpression final : public Expression
    {
    public:
        explicit DefaultGeomPropExpression(string name, Token token = {});

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        string name_;
        mx::GeomPropDefPtr geomprop_;
    };
}

#endif //MXSLC_DEFAULTGEOMPROPEXPRESSION_H
