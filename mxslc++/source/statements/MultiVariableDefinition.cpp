//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/Variable.h"
#include "runtime/values/Value.h"

MultiVariableDefinition::MultiVariableDefinition(const Runtime& runtime, vector<VariableDeclaration> decls, ExprPtr expr)
    : Statement{runtime}, decls_{std::move(decls)}, expr_{std::move(expr)}
{

}

StmtPtr MultiVariableDefinition::instantiate_template_types(const Type& template_type) const
{
    vector decls = Type::instantiate_template_types(decls_, template_type);
    ExprPtr expr = expr_->instantiate_template_types(template_type);
    return std::make_unique<MultiVariableDefinition>(runtime_, std::move(decls), std::move(expr));
}

void MultiVariableDefinition::execute()
{
    vector<Type> types;
    types.reserve(decls_.size());
    for (const VariableDeclaration& decl : decls_)
        types.push_back(decl.type);

    const Type type{std::move(types)};
    expr_->init(type);

    const ValuePtr value = expr_->evaluate();
    for (size_t i = 0; i < decls_.size(); i++)
    {
        ValuePtr subvalue = value->subvalue(i);
        Variable var{decls_[i], std::move(subvalue)};
        runtime_.scope().add_variable(std::move(var));
    }
}
