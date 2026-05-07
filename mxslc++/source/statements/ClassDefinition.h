//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_CLASSDEFINITION_H
#define MXSLC_CLASSDEFINITION_H

#include "Statement.h"
#include "runtime/Field.h"

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

    string name_;
    vector<TypePtr> template_types_;
    TypePtr parent_;
    vector<StmtPtr> body_;
};

#endif //MXSLC_CLASSDEFINITION_H
