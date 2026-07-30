//
// Created by jaket on 09/01/2026.
//

#ifndef MXSLC_MULTIVARIABLEDEFINITION_H
#define MXSLC_MULTIVARIABLEDEFINITION_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class MultiVariableDefinition final : public Statement
    {
    public:
        MultiVariableDefinition(TypePtr type, ExprPtr expr);
        MultiVariableDefinition(TypePtr type, ExprPtr expr, Token token);

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init() override;
        void execute_impl() const override;

    private:
        TypePtr type_;
        ExprPtr expr_;
    };
}

#endif //MXSLC_MULTIVARIABLEDEFINITION_H
