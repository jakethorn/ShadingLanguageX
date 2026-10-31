//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_FUNCTIONDEFINITION_H
#define MXSLC_FUNCTIONDEFINITION_H

#include "statements/Statement.h"
#include "runtime/ModifierList.h"
#include "runtime/ParameterList.h"

namespace mxslc::statements
{
    // Adds CompileOptions::default_function_behaviour to the modifiers of a function definition that has none of the
    // nodedef, [[nodegraph]], inline or comptime modifiers.
    void add_default_function_behaviour(ModifierList& mods);

    class FunctionDefinition final : public Statement
    {
    public:
        FunctionDefinition(
            ModifierList mods,
            TypePtr type,
            string name,
            vector<TypePtr> template_types,
            optional<ParameterList> params,
            StmtPtr body,
            Token token = {}
        );

        void set_attributes(AttributeList attrs) override;

        StmtPtr monomorphize(const TypePtr& template_type) const override;
        void create_functions();

        const vector<FuncPtr>& functions() const { return funcs_; }

        string to_string() const override;

    protected:
        void init() override;
        void execute_impl() const override;

    private:
        bool is_templated() const { return not template_types_.empty(); }
        void validate_template_types();

        ModifierList mods_;
        TypePtr type_;
        string name_;
        vector<TypePtr> template_types_;
        optional<ParameterList> params_;
        StmtPtr body_;

        vector<FuncPtr> funcs_;

        AttributeList attrs_;
    };
}

#endif //MXSLC_FUNCTIONDEFINITION_H
