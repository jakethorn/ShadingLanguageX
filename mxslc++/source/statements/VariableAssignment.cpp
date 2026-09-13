//
// Created by jaket on 27/03/2026.
//

#include "statements/VariableAssignment.h"

#include "expressions/Expression.h"
#include "expressions/Identifier.h"
#include "expressions/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/interface.h"
#include "runtime/variables/Variable.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/Serializer.h"
#include "serialize/serializer_utils.h"
#include "serialize/values/NodeValue.h"
#include "serialize/values/interface.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"
#include "statements/interface.h"

namespace mxslc::statements
{
    VariableAssignment::VariableAssignment(Token token, ExprPtr lhs_expr, ExprPtr rhs_expr)
        : Statement{std::move(token)}, lhs_expr_{std::move(lhs_expr)}, rhs_expr_{std::move(rhs_expr)}
    {

    }

    VariableAssignment::~VariableAssignment() = default;

    void VariableAssignment::set_attributes(AttributeList attrs)
    {
        rhs_expr_->set_attributes(std::move(attrs));
    }

    StmtPtr VariableAssignment::monomorphize(const TypePtr& template_type) const
    {
        auto&& [lhs, rhs] = runtime_utils::monomorphize_all(template_type, lhs_expr_, rhs_expr_);
        return create_statement<VariableAssignment>(token_, std::move(lhs), std::move(rhs));
    }

    void VariableAssignment::execute_impl() const
    {
        lhs_expr_->init();
        rhs_expr_->init(lhs_expr_->type());
        VarPtr rhs_val = rhs_expr_->evaluate();

        if (serializer().emit_source_hints())
        {
            if (const auto id = cast_expression<Identifier>(lhs_expr_))
            {
                const string var_name = id->name();
                mx::NodePtr rhs_node = serialize_utils::get_node(rhs_val);

                if (!rhs_node)
                {
                    // Literal value reassigned (e.g. i = 5;)
                    const mx::GraphElementPtr& graph = scope().graph();
                    if (graph)
                    {
                        rhs_node = graph->addNode("constant", graph->createValidChildName("var"), lhs_expr_->type()->name());
                        rhs_node->setAttribute("mxsl:literal", "true");
                        if (rhs_val && rhs_val->has_value())
                        {
                            const mx::InputPtr input = mtlx_utils::add_or_get_input(rhs_node, lhs_expr_->type()->name(), "value");
                            rhs_val->raw_value()->set_as_node_input(input);
                            input->setAttribute("mxsl:positional", "true");
                        }
                        rhs_val = create_variable(serialize::values::create_value<serialize::values::NodeValue>(rhs_node));
                    }
                }
                else if (!string_utils::starts_with(rhs_node->getName(), "var__"))
                {
                    // Existing named variable assigned (e.g. a = b;), insert dot passthrough node
                    const mx::GraphElementPtr& graph = scope().graph();
                    if (graph)
                    {
                        const mx::NodePtr dot_node = graph->addNode("dot", graph->createValidChildName("var"), lhs_expr_->type()->name());
                        const mx::InputPtr input = mtlx_utils::add_or_get_input(dot_node, lhs_expr_->type()->name(), "in");
                        input->setConnectedNode(rhs_node);
                        input->setAttribute("mxsl:positional", "true");
                        rhs_val = create_variable(serialize::values::create_value<serialize::values::NodeValue>(dot_node));
                        rhs_node = dot_node;
                    }
                }

                if (rhs_node)
                {
                    rhs_node->setAttribute("mxsl:assign", var_name);
                    serializer().tag_node(rhs_node);
                }
            }
        }

        lhs_expr_->assign(rhs_val);
    }

    string VariableAssignment::to_string() const
    {
        return lhs_expr_->to_string() + " = " + rhs_expr_->to_string() + ";";
    }
}
