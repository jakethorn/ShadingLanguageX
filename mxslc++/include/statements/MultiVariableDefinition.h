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

        TypePtr type() const;
        // the type as it is written, i.e., before it is resolved
        const TypePtr& declared_type() const { return type_; }
        const ExprPtr& expression() const { return expr_; }
        StmtPtr with_expression(ExprPtr expr) const;

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override;

    protected:
        void init() override;
        void execute_impl() const override;

    private:
        // e.g., `float a, b`
        string declaration_string() const;

        TypePtr type_;
        ExprPtr expr_;
        // the value as it is written in the skeleton, e.g., the code of a compile-time value
        mutable string hint_value_{"_"};
    };
}

#endif //MXSLC_MULTIVARIABLEDEFINITION_H
