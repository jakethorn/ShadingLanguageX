//
// Created by jaket on 14/05/2026.
//

#ifndef MXSLC_CONSTRUCTORDEFINITION_H
#define MXSLC_CONSTRUCTORDEFINITION_H

#include "statements/Statement.h"
#include "runtime/ModifierList.h"
#include "runtime/ParameterList.h"

namespace mxslc::statements
{
    class ConstructorDefinition final : public Statement
    {
    public:
        ConstructorDefinition(ModifierList mods, string class_name, ParameterList params, StmtPtr body, Token token);

        StmtPtr monomorphize(const TypePtr& template_type) const override;
        void init() override;

        const string& class_name() const { return class_name_; }

    protected:
        void execute_impl() const override;

    private:
        ModifierList mods_;
        string class_name_;
        ParameterList params_;
        StmtPtr body_;

        FuncPtr inner_ctor_;
        FuncPtr outer_ctor_;
    };
}

#endif //MXSLC_CONSTRUCTORDEFINITION_H
