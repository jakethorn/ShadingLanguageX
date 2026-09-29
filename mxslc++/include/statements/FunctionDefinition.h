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
        const string& name() const { return name_; }
        const vector<TypePtr>& template_types() const { return template_types_; }
        const AttributeList& attributes() const { return attrs_; }
        // only valid before the functions are created
        const StmtPtr& body() const { return body_; }
        StmtPtr release_body() { return std::move(body_); }
        // only valid before the functions are created
        const optional<ParameterList>& parameters() const { return params_; }
        // only valid before the functions are created
        bool is_inline() const { return mods_.contains(TokenType::Inline) or mods_.contains(TokenType::Comptime); }
        // a copy of an uninitialized definition with a different body
        StmtPtr with_body(StmtPtr body) const;

        string to_string() const override;
        bool is_block() const override { return true; }

        bool is_hinted() const override { return true; }
        string hint_skeleton() const override;

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
        string header_string_;
    };
}

#endif //MXSLC_FUNCTIONDEFINITION_H
