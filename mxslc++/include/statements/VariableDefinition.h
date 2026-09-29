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
        // the type as it is written, i.e., before it is resolved
        const TypePtr& declared_type() const { return type_; }
        const ExprPtr& expression() const { return expr_; }
        StmtPtr with_expression(ExprPtr expr) const;

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

        bool is_hinted() const override { return is_hinted_; }
        string hint_skeleton() const override;
        // variable definitions that are part of another statement are recorded by that statement
        void disable_hints() { is_hinted_ = false; }

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
        // the type as it is written, before it is resolved, e.g., an alias
        string type_string_;
        bool is_hinted_{true};
    };
}

#endif //FENNEC_VARIABLEDEFINITION_H
