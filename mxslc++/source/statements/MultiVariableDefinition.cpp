//
// Created by jaket on 09/01/2026.
//

#include "MultiVariableDefinition.h"
#include "expressions/Expression.h"

VariableDeclaration VariableDeclaration::instantiate_template_types(const Type& template_type) const
{
    return {modifiers, type.instantiate_template_types(template_type), name};
}

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
    
}
