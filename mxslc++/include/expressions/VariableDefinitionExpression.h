//
// Created by jaket on 13/04/2026.
//

#ifndef MXSLC_VARIABLEDEFINITIONEXPRESSION_H
#define MXSLC_VARIABLEDEFINITIONEXPRESSION_H

#include "expressions/Expression.h"
#include "runtime/ModifierList.h"

namespace mxslc::expressions
{
    class VariableDefinitionExpression final : public Expression
    {
    public:
        VariableDefinitionExpression(ModifierList mods, TypePtr type, Token name);
        VariableDefinitionExpression(StmtPtr var_def, ExprPtr identifier);

        ExprPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init_impl(const vector<TypePtr>& types) override;
        TypePtr type_impl() const override;
        VarPtr evaluate_impl() const override;

    private:
        const string& name() const { return token_.lexeme(); }

        StmtPtr var_def_;
        ExprPtr identifier_;
    };
}

#endif //MXSLC_VARIABLEDEFINITIONEXPRESSION_H
