//
// Created by jaket on 09/01/2026.
//

#include "statements/MultiVariableDefinition.h"
#include "expressions/Expression.h"
#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    MultiVariableDefinition::MultiVariableDefinition(TypePtr type, ExprPtr expr)
        : MultiVariableDefinition{std::move(type), std::move(expr), Token{}}
    {

    }

    MultiVariableDefinition::MultiVariableDefinition(TypePtr type, ExprPtr expr, Token token)
        : Statement{std::move(token)}, type_{std::move(type)}, expr_{std::move(expr)}
    {

    }

    void MultiVariableDefinition::set_attributes(AttributeList attrs)
    {
        if (expr_)
            expr_->set_attributes(std::move(attrs));
    }

    StmtPtr MultiVariableDefinition::monomorphize(const TypePtr& template_type) const
    {
        auto&& [type, expr] = runtime_utils::monomorphize_all(template_type, type_, expr_);
        return create_statement<MultiVariableDefinition>(std::move(type), std::move(expr), token_);
    }

    void MultiVariableDefinition::execute_impl() const
    {
        const TypePtr type = scope().resolve_type(type_);

        VarPtr value;
        if (expr_)
        {
            expr_->init(type);
            value = expr_->evaluate();
        }
        else
        {
            value = create_variable(type);
        }

        for (size_t i = 0; i < value->child_count(); ++i)
        {
            Field field = type->field(i);
            VarPtr child = value->child(i);
            if (field.is_global())
            {
                if (VarPtr global = runtime().global(field.name()))
                {
                    const ExprPtr child_expr = create_expression<RuntimeExpression>(std::move(global));
                    child_expr->init(type);
                    child = child_expr->evaluate();
                }
            }
            const VarPtr var = create_variable(field.modifiers(), field.type(), child);
            var->add_to_scope(type->field_name(i));
        }
    }

    string MultiVariableDefinition::to_string() const
    {
        string result = join(type_->fields(), ", ");
        if (expr_)
            result += " = " + expr_->to_string();
        result += ";";
        return result;
    }
}
