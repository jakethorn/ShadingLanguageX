//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_NAMEDCONSTRUCTOR_H
#define MXSLC_NAMEDCONSTRUCTOR_H

#include "Expression.h"
#include "runtime/ArgumentList.h"

class NamedConstructor final : public Expression
{
public:
    NamedConstructor(Token name, ArgumentList args);

    ExprPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void init_impl(const vector<TypePtr>& types) override;
    TypePtr type_impl() const override;
    VarPtr evaluate_impl() const override;

private:
    ArgumentList args_;

    ExprPtr func_call_;
};

#endif //MXSLC_NAMEDCONSTRUCTOR_H