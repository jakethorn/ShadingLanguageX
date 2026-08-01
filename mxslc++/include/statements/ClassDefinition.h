//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_CLASSDEFINITION_H
#define MXSLC_CLASSDEFINITION_H

#include "statements/Statement.h"

namespace mxslc::statements
{
    class FunctionDefinition;
    class ConstructorDefinition;

    class ClassDefinition final : public Statement
    {
    public:
        ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body);
        ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body, Token token);

        StmtPtr monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    protected:
        void execute_impl() const override;

    private:
        bool is_templated() const { return not template_types_.empty(); }
        void validate_body() const;
        void add_fields(const TypePtr& class_type) const;
        void add_methods_and_constructors(const TypePtr& class_type) const;
        void add_method(const TypePtr& class_type, FunctionDefinition* func_def) const;
        void add_constructor(const TypePtr& class_type, ConstructorDefinition* ctor_def) const;

        string name_;
        vector<TypePtr> template_types_;
        TypePtr parent_;
        vector<StmtPtr> body_;
    };
}

#endif //MXSLC_CLASSDEFINITION_H
