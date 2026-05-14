//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_CLASSDEFINITION_H
#define MXSLC_CLASSDEFINITION_H

#include "Statement.h"

class FunctionDefinition;
class ConstructorDefinition;

class ClassDefinition final : public Statement
{
public:
    ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body);
    ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body, Token token);

    StmtPtr instantiate_template_types(const TypePtr& template_type) const override;

protected:
    void execute_impl() const override;

private:
    bool is_templated() const { return not template_types_.empty(); }
    void validate_body() const;
    void add_fields(const TypePtr& type) const;
    void add_methods_and_constructors(const TypePtr& type) const;
    void add_method(const TypePtr& type, FunctionDefinition* func_def) const;
    void add_constructor(const TypePtr& type, ConstructorDefinition* ctor_def) const;

    string name_;
    vector<TypePtr> template_types_;
    TypePtr parent_;
    vector<StmtPtr> body_;
};

#endif //MXSLC_CLASSDEFINITION_H
