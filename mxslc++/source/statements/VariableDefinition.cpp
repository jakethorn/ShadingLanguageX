//
// Created by jaket on 28/11/2025.
//

#include "statements/VariableDefinition.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "expressions/interface.h"
#include "expressions/RuntimeExpression.h"
#include "runtime/interface.h"
#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/invoke.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/Serializer.h"
#include "serialize/serializer_utils.h"
#include "serialize/values/NodeValue.h"
#include "serialize/values/interface.h"
#include "utils/mtlx_utils.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr)
        : VariableDefinition{std::move(mods), std::move(type), std::move(name), std::move(expr), Token{}}
    {

    }

    VariableDefinition::VariableDefinition(ModifierList mods, TypePtr type, string name, ExprPtr expr, Token token)
        : Statement{std::move(token)}, mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, expr_{std::move(expr)}
    {

    }

    VariableDefinition::~VariableDefinition() = default;

    const ModifierList &VariableDefinition::modifiers() const
    {
        return mods_;
    }

    TypePtr VariableDefinition::type() const
    {
        return scope().resolve_type(type_);
    }

    const string& VariableDefinition::name() const
    {
        return name_;
    }

    void VariableDefinition::set_attributes(AttributeList attrs)
    {
        attrs_ = attrs;
        if (expr_)
            expr_->set_attributes(std::move(attrs));
    }

    StmtPtr VariableDefinition::monomorphize(const TypePtr& template_type) const
    {
        auto&& [type, expr] = runtime_utils::monomorphize_all(template_type, type_, expr_);
        return create_statement<VariableDefinition>(mods_, std::move(type), name_, std::move(expr), token_);
    }

    void VariableDefinition::init()
    {
        type_ = scope().resolve_type(type_);
    }

    void VariableDefinition::execute_impl() const
    {
        const bool is_global = mods_.contains(TokenType::Global);
        const bool is_geomprop = mods_.contains(TokenType::Geomprop);

        VarPtr value;
        if (is_global)
        {
            value = evaluate_global();
        }

        if (not value and expr_)
        {
            expr_->init(type_);
            value = expr_->evaluate();
        }

        if (is_geomprop)
            value = evaluate_geomprop(value);

        if (not value)
            value = create_variable(type_);

        if (serializer().emit_source_hints())
        {
            const bool is_mutable = mods_.contains(TokenType::Mutable);
            const bool is_const = mods_.contains(TokenType::Const);
            if (is_mutable || is_const)
            {
                mx::NodePtr node = serialize_utils::get_node(value);
                if (!node && value->has_value())
                {
                    const mx::GraphElementPtr& graph = scope().graph();
                    if (graph)
                    {
                        node = graph->addNode("constant", graph->createValidChildName(name_), type_->name());
                        node->setAttribute("mxsl:literal", "true");
                        const mx::InputPtr input = mtlx_utils::add_or_get_input(node, type_->name(), "value");
                        value->raw_value()->set_as_node_input(input);
                        input->setAttribute("mxsl:positional", "true");
                        value = create_variable(serialize::values::create_value<serialize::values::NodeValue>(node));
                    }
                }
                if (node)
                {
                    if (is_mutable)
                        node->setAttribute("mxsl:mutable", "true");
                    if (is_const)
                        node->setAttribute("mxsl:const", "true");
                    serializer().tag_node(node);
                }
            }
        }

        const VarPtr var = create_variable(mods_.without(TokenType::Global, TokenType::Geomprop), type_, value);
        var->add_to_scope(name_);
    }

    VarPtr VariableDefinition::evaluate_global() const
    {
        if (VarPtr global = runtime().global(name_))
        {
            // init and evaluate here for type checking
            const ExprPtr value_expr = as_expression(std::move(global));
            value_expr->init(type_);
            return value_expr->evaluate();
        }

        return nullptr;
    }

    VarPtr VariableDefinition::evaluate_geomprop(VarPtr default_value) const
    {
        ArgumentList args{name_};
        if (default_value)
            args.add(std::move(default_value));
        return runtime_utils::invoke_function(type_, "geompropvalue", std::move(args));
    }

    string VariableDefinition::to_string() const
    {
        string attrs_string = attrs_.to_string();
        if (not attrs_string.empty())
            attrs_string += '\n';

        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string result = attrs_string + mods_string;
        result += type_->to_string() + ' ' + name_;
        if (expr_)
            result += " = " + expr_->to_string();
        result += ';';

        return result;
    }
}
