//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_NAMEDCONSTRUCTOR_H
#define MXSLC_NAMEDCONSTRUCTOR_H

#include "expressions/Expression.h"
#include "runtime/ArgumentList.h"

namespace mxslc::expressions
{
    class NamedConstructor final : public Expression
    {
    public:
        NamedConstructor(string name, ArgumentList args);
        NamedConstructor(string name, ArgumentList args, Token token);

        ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        string name_;
        ArgumentList args_;

        ExprPtr func_call_;
    };
}

#endif //MXSLC_NAMEDCONSTRUCTOR_H
