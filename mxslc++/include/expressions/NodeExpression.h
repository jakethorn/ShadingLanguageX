//
// Created by jaket on 17/11/2026.
//

#ifndef MXSLC_NODEEXPRESSION_H
#define MXSLC_NODEEXPRESSION_H

#include "expressions/Expression.h"
#include "common.h"
#include "runtime/ArgumentList.h"

namespace mxslc::expressions
{
    class NodeExpression : public Expression
    {
    public:
        NodeExpression(string category, TypePtr template_type, ArgumentList args, Token token);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        void init_subexpressions(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        string category_;
        TypePtr template_type_;
        ArgumentList args_;

        TypePtr node_type_;
    };
}

#endif //MXSLC_NODEEXPRESSION_H
