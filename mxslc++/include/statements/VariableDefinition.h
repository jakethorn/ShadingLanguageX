//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_VARIABLEDEFINITION_H
#define FENNEC_VARIABLEDEFINITION_H

#include "statements/Statement.h"
#include "runtime/ModifierList.h"

namespace mxslc::statements
{
    class VariableDefinition final : public Statement
    {
    public:
        VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr);
        VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr, Token token);
        ~VariableDefinition() override;

        const ModifierList& modifiers() const;
        TypePtr type() const;
        const string& name() const;

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void init() override;
        void execute_impl() const override;

    private:
        VarPtr evaluate_global() const;
        VarPtr evaluate_geomprop(VarPtr default_value) const;

        ModifierList mods_;
        TypePtr type_;
        string name_;
        ExprPtr expr_;
    };
}

#endif //FENNEC_VARIABLEDEFINITION_H
