//
// Created by jaket on 13/04/2026.
//

#include "expressions/VariableDefinitionExpression.h"

#include "statements/VariableDefinition.h"
#include "expressions/Identifier.h"
#include "expressions/interface.h"
#include "runtime/Variable.h"
#include "statements/interface.h"

namespace mxslc::expressions
{
    VariableDefinitionExpression::VariableDefinitionExpression(ModifierList mods, TypePtr type, Token name)
        : VariableDefinitionExpression{
            create_statement<VariableDefinition>(std::move(mods), std::move(type), name.lexeme(), nullptr),
            create_expression<Identifier>(name)
        }
    {

    }

    VariableDefinitionExpression::VariableDefinitionExpression(StmtPtr var_def, ExprPtr identifier)
        : Expression{var_def->token()}, var_def_{std::move(var_def)}, identifier_{std::move(identifier)}
    {

    }

    ExprPtr VariableDefinitionExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<VariableDefinitionExpression>(
            var_def_->monomorphize(template_type),
            identifier_->monomorphize(template_type)
        );
    }

    void VariableDefinitionExpression::init_impl(const vector<TypePtr>& types)
    {
        var_def_->execute();
        identifier_->init(types);
    }

    TypePtr VariableDefinitionExpression::type_impl() const
    {
        return identifier_->type();
    }

    VarPtr VariableDefinitionExpression::evaluate_impl() const
    {
        VarPtr var = identifier_->evaluate();
        var->uninitialize();
        return var;
    }

    string VariableDefinitionExpression::to_string() const
    {
        return var_def_->to_string() + " " + identifier_->to_string();
    }
}
