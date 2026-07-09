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

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        const ModifierList& modifiers() const { return mods_; }
        TypePtr type() const;
        const string& name() const;

    protected:
        void execute_impl() const override;

    private:
        ModifierList mods_;
        TypePtr type_;
        string name_;
        ExprPtr expr_;
    };
}

#endif //FENNEC_VARIABLEDEFINITION_H
