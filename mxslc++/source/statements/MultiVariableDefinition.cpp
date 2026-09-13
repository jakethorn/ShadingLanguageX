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
#include "CompileOptions.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeOutputValue.h"

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

        if (runtime().options().emit_source_hints && value)
        {
            mx::NodePtr node = nullptr;
            bool all_same_node = (value->child_count() > 0);
            for (size_t i = 0; i < value->child_count(); ++i)
            {
                if (const auto node_output = serialize::values::cast_value<serialize::values::NodeOutputValue>(value->child(i)->raw_value()))
                {
                    if (node == nullptr)
                        node = node_output->node();
                    else if (node != node_output->node())
                    {
                        all_same_node = false;
                        break;
                    }
                }
                else
                {
                    all_same_node = false;
                    break;
                }
            }

            if (all_same_node && node != nullptr)
            {
                node->setAttribute("mxsl:multivar", "true");
                string varnames;
                for (size_t i = 0; i < type_->field_count(); ++i)
                {
                    if (i > 0)
                        varnames += ",";
                    varnames += type_->field(i).name();
                }
                node->setAttribute("mxsl:varnames", varnames);
            }
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
        string result;
        for (size_t i = 0; i < type_->field_count(); ++i)
        {
            if (i > 0)
                result += ", ";

            const Field& field = type_->field(i);
            const bool same_as_prev = (i > 0) &&
                                      (field.type()->to_string() == type_->field(i - 1).type()->to_string()) &&
                                      (field.modifiers() == type_->field(i - 1).modifiers());

            if (same_as_prev)
            {
                result += field.name();
            }
            else
            {
                string mods_str = field.modifiers().to_string();
                if (!mods_str.empty())
                    result += mods_str + " ";
                result += field.type()->to_string() + " " + field.name();
            }
        }

        if (expr_)
            result += " = " + expr_->to_string();
        result += ";";
        return result;
    }
}
