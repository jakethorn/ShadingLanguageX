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
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "statements/interface.h"
#include "statements/VariableDefinition.h"

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

    TypePtr MultiVariableDefinition::type() const
    {
        return scope().resolve_type(type_);
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

    void MultiVariableDefinition::init()
    {
        type_ = scope().resolve_type(type_);
    }

    void MultiVariableDefinition::execute_impl() const
    {
        VarPtr value;
        if (expr_)
        {
            expr_->init(type_);
            value = expr_->evaluate();
        }

        for (size_t i = 0; i < type_->field_count(); ++i)
        {
            Field field = type_->field(i);

            ExprPtr child_expr;
            if (value)
            {
                VarPtr child = value->child(i);
                child_expr = as_expression(std::move(child));
            }

            create_statement<VariableDefinition>(
                field.modifiers(),
                field.type(),
                field.name(),
                std::move(child_expr)
            )->execute();
        }
    }

    string MultiVariableDefinition::to_string() const
    {
        const vector<Field>& fields = type_->fields();

        // `float a, b` if all fields share the same modifiers and type, otherwise `float a, int b`
        bool is_uniform = true;
        for (const Field& field : fields)
        {
            if (field.modifiers().to_string() != fields.front().modifiers().to_string() or
                field.type()->to_string() != fields.front().type()->to_string())
                is_uniform = false;
        }

        string result;
        for (size_t i = 0; i < fields.size(); ++i)
        {
            if (i > 0)
                result += ", ";

            if (i == 0 or not is_uniform)
            {
                const string mods_string = fields[i].modifiers().to_string();
                if (not mods_string.empty())
                    result += mods_string + ' ';
                result += fields[i].type()->to_string() + ' ';
            }

            result += fields[i].name();
        }

        if (expr_)
            result += " = " + expr_->to_string();
        result += ";";

        return with_attributes(expr_ ? expr_->attributes() : AttributeList{}, result);
    }
}
