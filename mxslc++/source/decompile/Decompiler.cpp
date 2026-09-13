//
// Created by jaket on 19/06/2026.
//

#include "decompile/Decompiler.h"

#include <MaterialXFormat/XmlIo.h>
#include <sstream>

#include "TokenType.h"
#include "common.h"
#include "decompile/NodeLifter.h"
#include "errors/CompileError.h"
#include "expressions/DotOperator.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IfExpression.h"
#include "expressions/IndexingOperator.h"
#include "expressions/Literal.h"
#include "expressions/NamedConstructor.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/interface.h"
#include "runtime/Type.h"
#include "runtime/Field.h"
#include "runtime/interface.h"
#include "serialize/serialize_name_utils.h"
#include "statements/BlockStatement.h"
#include "statements/FunctionDefinition.h"
#include "statements/ReturnStatement.h"
#include "statements/VariableDefinition.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/interface.h"
#include "utils/container_utils.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace expressions;
    using namespace statements;
    using namespace runtime;
    using string_utils::starts_with;
    using namespace container_utils;

    namespace
    {
        string safe_mxsl_name(const vector<mx::OutputPtr>& outputs, const string& name)
        {
            const TokenType type{name};
            if (type == TokenType::Unknown or type == TokenType::Identifier)
                return name;
            for (size_t i = 0; i < outputs.size(); ++i)
            {
                if (outputs[i]->getName() == name)
                    return "out" + std::to_string(i + 1);
            }
            return name;
        }

        string node_graph_output_field_name(const vector<mx::OutputPtr>& outputs, const string& output_name)
        {
            return safe_mxsl_name(outputs, serialize::remove_prefix(output_name));
        }

        bool is_separate_category(const string& category)
        {
            return category == "separate" or category == "separate2"
                or category == "separate3" or category == "separate4";
        }

        int separate_output_index(const mx::NodePtr& node, const string& output)
        {
            const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node);
            if (node_def)
            {
                const vector<mx::OutputPtr> outputs = node_def->getActiveOutputs();
                for (size_t i = 0; i < outputs.size(); ++i)
                {
                    if (outputs[i]->getName() == output)
                        return static_cast<int>(i);
                }
            }
            throw CompileError{"Cannot determine output index for '" + output +
                               "' of separate node '" + node->getName() + "'"};
        }

        vector<string> split_varnames(const string& str)
        {
            vector<string> result;
            std::istringstream stream(str);
            string token;
            while (std::getline(stream, token, ','))
            {
                if (!token.empty())
                    result.push_back(token);
            }
            return result;
        }
    }

    Decompiler::Decompiler(const fs::path& src_path)
    {
        document_ = mx::createDocument();
        mx::readFromXmlFile(document_, src_path.string());
    }

    Decompiler::Decompiler(const string& source)
    {
        document_ = mx::createDocument();
        mx::readFromXmlString(document_, source);
    }

    Decompiler::Decompiler(mx::DocumentPtr document) : document_{std::move(document)}
    {

    }

    void Decompiler::analyze_graph(const mx::ElementPtr& parent)
    {
        ref_counts_.clear();
        consumed_nodes_.clear();

        const vector<mx::NodePtr> nodes = parent->getChildrenOfType<mx::Node>();

        // 1. Identify swizzles and mark separate nodes as consumed
        for (const auto& node : nodes)
        {
            string swiz_channels;
            mx::InputPtr src_in;
            if (NodeLifter::is_swizzle(node, swiz_channels, src_in))
            {
                if (const auto separate_node = src_in->getParent()->asA<mx::Node>())
                    consumed_nodes_.insert(separate_node);
            }
        }

        // 2. Count references to each node
        for (const auto& node : nodes)
        {
            for (const auto& input : node->getInputs())
            {
                if (input->hasNodeName())
                {
                    if (mx::NodePtr connected = parent->getChildOfType<mx::Node>(input->getNodeName()))
                        ref_counts_[connected]++;
                }
            }
        }

        if (const auto nodegraph = parent->asA<mx::NodeGraph>())
        {
            for (const auto& output : nodegraph->getOutputs())
            {
                if (output->hasNodeName())
                {
                    if (mx::NodePtr connected = parent->getChildOfType<mx::Node>(output->getNodeName()))
                        ref_counts_[connected]++;
                }
            }
        }
    }

    bool Decompiler::is_inline_node(const mx::NodePtr& node) const
    {
        if (consumed_nodes_.count(node))
            return true;

        if (node->getAttribute("mxsl:multivar") == "true")
            return false;

        if (node->getName().rfind("var__", 0) == 0)
        {
            const auto it = ref_counts_.find(node);
            if (it != ref_counts_.end() && it->second > 1)
                return false;
            return true;
        }

        return false;
    }

    bool Decompiler::is_swizzle_consumed(const mx::NodePtr& node) const
    {
        return consumed_nodes_.count(node) > 0;
    }

    string Decompiler::decompile_document()
    {
        global_code_ = "";
        decompiled_nodes_.clear();
        decompiled_node_graphs_.clear();
        node_graph_var_names_.clear();

        // Document-level metadata, e.g. `@@doc "..."`
        for (const string& attr_name : document_->getAttributeNames())
        {
            if (attr_name != "version" && !NodeLifter::is_structural_attribute(attr_name))
                global_code_ += "@@" + attr_name + " \"" + document_->getAttribute(attr_name) + "\"\n";
        }

        for (const mx::NodeGraphPtr& node_graph : document_->getNodeGraphs())
        {
            global_code_ += node_graph_to_function_definition(node_graph);
        }

        analyze_graph(document_);
        current_statements_.clear();
        for (const mx::NodePtr& node : document_->getNodes())
        {
            emit_node(node, current_statements_);
        }

        for (const StmtPtr& stmt : current_statements_)
        {
            global_code_ += stmt->to_string() + "\n";
        }

        return global_code_;
    }

    string Decompiler::decompile_node(const string& node_name, const bool with_dependencies)
    {
        return decompile_node(document_->getNode(node_name), with_dependencies);
    }

    string Decompiler::decompile_node(const mx::NodePtr& node, const bool with_dependencies)
    {
        global_code_ = "";
        decompiled_nodes_.clear();
        analyze_graph(document_);

        current_statements_.clear();
        emit_node(node, current_statements_);

        string code;
        for (const StmtPtr& s : current_statements_)
            code += s->to_string() + "\n";

        return code;
    }

    string Decompiler::decompile_node_def(const string& node_def_name, const bool with_dependencies)
    {
        return decompile_node_def(document_->getNodeDef(node_def_name), with_dependencies);
    }

    string Decompiler::decompile_node_def(const mx::NodeDefPtr& node_def, const bool with_dependencies)
    {
        global_code_ = "";
        decompiled_nodes_.clear();

        string code = node_def_to_function_definition(node_def);
        if (with_dependencies)
            code = global_code_ + code;
        return code;
    }

    string Decompiler::decompile_node_graph(const string& node_graph_name, const bool with_dependencies)
    {
        return decompile_node_graph(document_->getNodeGraph(node_graph_name), with_dependencies);
    }

    string Decompiler::decompile_node_graph(const mx::NodeGraphPtr& node_graph, const bool with_dependencies)
    {
        global_code_ = "";
        decompiled_nodes_.clear();

        string code = node_graph_to_function_definition(node_graph);
        if (with_dependencies)
            code = global_code_ + code;
        return code;
    }

    string Decompiler::node_to_variable_definition(const string& node_name)
    {
        return node_to_variable_definition(document_->getNode(node_name));
    }

    string Decompiler::node_to_variable_definition(const mx::NodePtr& node)
    {
        vector<StmtPtr> stmts;
        emit_node(node, stmts);
        string result;
        for (const StmtPtr& s : stmts)
            result += s->to_string() + "\n";
        return result;
    }

    string Decompiler::node_def_to_function_definition(const string& node_def_name)
    {
        return node_def_to_function_definition(document_->getNodeDef(node_def_name));
    }

    string Decompiler::node_def_to_function_definition(const mx::NodeDefPtr& node_def)
    {
        for (const mx::NodeGraphPtr& node_graph : document_->getNodeGraphs())
        {
            if (node_graph->getNodeDef() == node_def)
                return node_graph_to_function_definition(node_graph);
        }
        throw CompileError{"Cannot decompile NodeDef: " + node_def->getName()};
    }

    string Decompiler::node_graph_to_function_definition(const string& node_graph_name)
    {
        return node_graph_to_function_definition(document_->getNodeGraph(node_graph_name));
    }

    string Decompiler::node_graph_to_function_definition(const mx::NodeGraphPtr& node_graph)
    {
        if (contains(decompiled_node_graphs_, node_graph->getName()))
            return "";
        decompiled_node_graphs_.insert(node_graph->getName());

        analyze_graph(node_graph);

        const bool prev_in_func = in_function_;
        vector<StmtPtr> prev_stmts = std::move(current_statements_);
        in_function_ = true;
        current_statements_.clear();

        for (const mx::NodePtr& node : node_graph->getNodes())
        {
            emit_node(node, current_statements_);
        }

        const ExprPtr return_expr = get_node_graph_return_expression(node_graph);
        current_statements_.push_back(create_statement<ReturnStatement>(return_expr));

        const StmtPtr body_block = create_statement<BlockStatement>(std::move(current_statements_));

        in_function_ = prev_in_func;
        current_statements_ = std::move(prev_stmts);

        const string signature = get_node_graph_signature(node_graph);
        const string attrs = node_graph->hasNodeDefString() ? node_def_to_attributes(node_graph->getNodeDef()) : "";

        const string func_def = "\n" + attrs + signature + "\n" + body_block->to_string() + "\n";

        // If the nodegraph has interface inputs, also emit an instance variable
        const vector<mx::InputPtr> inputs = node_graph->getInputs();
        string var_def;
        if (!inputs.empty())
        {
            const string func_name = get_node_graph_identifier(node_graph);
            const string var_name = func_name + "_out";
            const string var_type = outputs_to_data_type(node_graph->getOutputs());
            const string args = inputs_to_arguments(inputs);
            var_def = var_type + " " + var_name + " = " + func_name + "(" + args + ");\n";
            node_graph_var_names_[node_graph->getName()] = var_name;
        }

        return func_def + var_def;
    }

    void Decompiler::emit_node(const mx::NodePtr& node, vector<StmtPtr>& target_stmts)
    {
        if (!node || contains(decompiled_nodes_, node) || is_swizzle_consumed(node) || is_inline_node(node))
            return;

        if (contains(resolving_nodes_, node))
            throw CompileError{"Cyclic dependency detected in node: " + node->getName()};

        resolving_nodes_.insert(node);

        // Resolve dependencies first
        for (const mx::InputPtr& inp : node->getInputs())
        {
            if (inp->hasNodeName())
            {
                const mx::ElementPtr parent = node->getParent();
                if (parent)
                {
                    if (const mx::NodePtr dep = parent->getChildOfType<mx::Node>(inp->getNodeName()))
                        emit_node(dep, target_stmts);
                }
            }
        }

        resolving_nodes_.erase(node);

        if (contains(decompiled_nodes_, node))
            return;
        decompiled_nodes_.insert(node);

        StmtPtr var_def = build_node_variable_definition(node);
        if (var_def)
            target_stmts.push_back(std::move(var_def));
    }

    StmtPtr Decompiler::build_node_variable_definition(const mx::NodePtr& node)
    {
        if (node->getAttribute("mxsl:multivar") == "true")
            return build_multivar_definition(node);

        const string var_type_str = get_node_data_type(node);
        const TypePtr type = create_type(var_type_str);
        ExprPtr expr = lift_node(node);

        StmtPtr var_def = create_statement<VariableDefinition>(ModifierList{}, type, node->getName(), std::move(expr));
        var_def->set_attributes(NodeLifter::extract_attributes(node));
        return var_def;
    }

    StmtPtr Decompiler::build_multivar_definition(const mx::NodePtr& node)
    {
        const string varnames_attr = node->getAttribute("mxsl:varnames");
        const vector<string> varnames = split_varnames(varnames_attr);

        vector<Field> fields;
        const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node);
        vector<mx::OutputPtr> outputs = node_def ? node_def->getActiveOutputs() : vector<mx::OutputPtr>{};
        if (outputs.empty())
            outputs = node->getOutputs();

        for (size_t i = 0; i < varnames.size(); ++i)
        {
            string type_str = "float";
            if (i < outputs.size())
                type_str = NodeLifter::get_type_alias(outputs[i]->getType());
            fields.emplace_back(create_type(type_str), varnames[i]);
        }

        const TypePtr multi_type = create_type(std::move(fields));
        ExprPtr expr;
        if (is_separate_category(node->getCategory()) && node->getInput("in"))
            expr = port_to_expression(node->getInput("in"));
        else
            expr = lift_node(node);

        StmtPtr multi_var_def = create_statement<MultiVariableDefinition>(multi_type, std::move(expr));
        multi_var_def->set_attributes(NodeLifter::extract_attributes(node));
        return multi_var_def;
    }

    string Decompiler::get_multivar_output_name(const mx::NodePtr& node, const string& output)
    {
        const string varnames_attr = node->getAttribute("mxsl:varnames");
        const vector<string> varnames = split_varnames(varnames_attr);
        if (varnames.empty())
            return "";

        if (is_separate_category(node->getCategory()))
        {
            const int idx = separate_output_index(node, output);
            if (idx >= 0 && static_cast<size_t>(idx) < varnames.size())
                return varnames[idx];
        }

        const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node);
        vector<mx::OutputPtr> outputs = node_def ? node_def->getActiveOutputs() : vector<mx::OutputPtr>{};
        if (outputs.empty())
            outputs = node->getOutputs();

        for (size_t i = 0; i < outputs.size() && i < varnames.size(); ++i)
        {
            if (outputs[i]->getName() == output)
                return varnames[i];
        }

        return "";
    }

    ExprPtr Decompiler::lift_node(const mx::NodePtr& node)
    {
        return NodeLifter::lift_node(node, [this](const mx::PortElementPtr& p) {
            return port_to_expression(p);
        });
    }

    ExprPtr Decompiler::port_to_expression(const mx::PortElementPtr& port)
    {
        if (port->hasValue())
            return NodeLifter::value_to_expression(port->getValue());

        if (port->hasInterfaceName())
            return create_expression<Identifier>(port->getInterfaceName());

        if (port->hasNodeName())
        {
            const mx::NodePtr node = port->getConnectedNode();
            if (port->hasOutputString())
                return node_and_output_to_dot_op(node, port->getOutputString());

            if (node->isMultiOutputType())
            {
                const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node);
                const vector<mx::OutputPtr> outputs = node_def ? node_def->getActiveOutputs() : vector<mx::OutputPtr>{};
                if (!outputs.empty())
                    return node_and_output_to_dot_op(node, outputs[0]->getName());
            }

            if (is_inline_node(node))
                return lift_node(node);

            emit_node(node, current_statements_);
            return create_expression<Identifier>(node->getName());
        }

        if (port->hasNodeGraphString())
        {
            if (port->hasOutputString())
                return node_graph_name_and_output_to_dot_op(port->getNodeGraphString(), port->getOutputString());
            return create_expression<Identifier>(port->getNodeGraphString());
        }

        throw CompileError{"Cannot decompile PortElement: " + port->asString()};
    }

    ExprPtr Decompiler::node_and_output_to_dot_op(const mx::NodePtr& node, const string& output)
    {
        string swiz_channels;
        mx::InputPtr swiz_in;
        if (NodeLifter::is_swizzle(node, swiz_channels, swiz_in))
            return lift_node(node);

        char extract_chan = 0;
        mx::InputPtr extract_in;
        if (NodeLifter::is_extract_swizzle(node, extract_chan, extract_in))
            return lift_node(node);

        if (node->getAttribute("mxsl:multivar") == "true")
        {
            emit_node(node, current_statements_);
            const string var_name = get_multivar_output_name(node, output);
            if (!var_name.empty())
                return create_expression<Identifier>(var_name);
        }

        if (is_separate_category(node->getCategory()))
        {
            const int idx = separate_output_index(node, output);
            ExprPtr base = is_inline_node(node) ? lift_node(node) : create_expression<Identifier>(node->getName());
            return create_expression<IndexingOperator>(std::move(base), create_expression<Literal>(Primitive{idx}));
        }

        ExprPtr base;
        if (is_inline_node(node))
            base = lift_node(node);
        else
        {
            emit_node(node, current_statements_);
            base = create_expression<Identifier>(node->getName());
        }

        return create_expression<DotOperator>(std::move(base), Token{TokenType::Identifier, output});
    }

    ExprPtr Decompiler::node_graph_name_and_output_to_dot_op(const string& node_graph_name, const string& output)
    {
        const mx::NodeGraphPtr node_graph = document_->getNodeGraph(node_graph_name);
        const vector<mx::OutputPtr> node_graph_outputs = node_graph ? node_graph->getOutputs() : vector<mx::OutputPtr>{};
        const string safe_output = node_graph_output_field_name(node_graph_outputs, output);

        string id_name;
        if (contains(node_graph_var_names_, node_graph_name))
            id_name = node_graph_var_names_.at(node_graph_name);
        else if (starts_with(node_graph_name, "NG_"))
            id_name = node_graph_name.substr(3);
        else
            id_name = node_graph_name;

        if (node_graph_outputs.size() == 1)
            return create_expression<Identifier>(id_name);

        return create_expression<DotOperator>(create_expression<Identifier>(id_name), Token{TokenType::Identifier, safe_output});
    }

    string Decompiler::node_to_attributes(const mx::NodePtr& node)
    {
        const AttributeList attrs = NodeLifter::extract_attributes(node);
        string result = attrs.to_string();
        if (!result.empty())
            result += "\n";
        return result;
    }

    string Decompiler::node_def_to_attributes(const mx::NodeDefPtr& node_def)
    {
        string result;
        for (const string& attr_name : node_def->getAttributeNames())
        {
            if (attr_name == mx::NodeDef::NODE_ATTRIBUTE || NodeLifter::is_structural_attribute(attr_name))
                continue;
            result += "@" + attr_name + " \"" + node_def->getAttribute(attr_name) + "\"\n";
        }
        return result;
    }

    string Decompiler::outputs_to_data_type(const vector<mx::OutputPtr>& outputs)
    {
        if (outputs.size() == 1)
        {
            return NodeLifter::get_type_alias(outputs[0]->getType());
        }
        else
        {
            string result = "{";
            for (const mx::OutputPtr& output : outputs)
            {
                if (serialize::has_prefix(output->getName(), OUT_PARAMETER_PREFIX) ||
                    serialize::has_prefix(output->getName(), NONLOCAL_OUT_PREFIX))
                    continue;
                result += NodeLifter::get_type_alias(output) + " " + node_graph_output_field_name(outputs, output->getName()) + ", ";
            }
            if (result.size() >= 2 && result.back() == ' ')
            {
                result.pop_back();
                result.pop_back();
            }
            return result + "}";
        }
    }

    string Decompiler::input_to_argument(const mx::InputPtr& input)
    {
        const AttributeList attrs = NodeLifter::extract_attributes(input);
        string result;
        for (const auto& attr : attrs.attributes())
            result += attr.to_string() + " ";

        const ExprPtr expr = port_to_expression(input);
        if (input->getAttribute("mxsl:positional") == "true")
            return result + expr->to_string();
        return result + input->getName() + " = " + expr->to_string();
    }

    string Decompiler::inputs_to_arguments(const vector<mx::InputPtr>& inputs)
    {
        string result;
        for (const mx::InputPtr& input : inputs)
            result += input_to_argument(input) + ", ";
        if (result.size() >= 2 && result.back() == ' ')
        {
            result.pop_back();
            result.pop_back();
        }
        return result;
    }

    string Decompiler::input_to_parameter(const mx::InputPtr& input)
    {
        const AttributeList attrs = NodeLifter::extract_attributes(input);
        string result;
        for (const auto& attr : attrs.attributes())
            result += attr.to_string() + " ";

        result += NodeLifter::get_type_alias(input->getType()) + " " + input->getName();
        if (input->hasValue())
            result += " = " + NodeLifter::value_to_expression(input->getValue())->to_string();

        return result;
    }

    string Decompiler::inputs_to_parameters(const vector<mx::InputPtr>& inputs)
    {
        string result;
        for (const mx::InputPtr& input : inputs)
            result += input_to_parameter(input) + ", ";
        if (result.size() >= 2 && result.back() == ' ')
        {
            result.pop_back();
            result.pop_back();
        }
        return result;
    }

    string Decompiler::get_node_data_type(const mx::NodePtr& node)
    {
        if (node->isMultiOutputType())
        {
            const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node);
            return outputs_to_data_type(node_def->getActiveOutputs());
        }
        return NodeLifter::get_type_alias(node);
    }

    string Decompiler::get_node_def_data_type(const mx::NodeDefPtr& node_def)
    {
        if (node_def->isMultiOutputType())
            return outputs_to_data_type(node_def->getActiveOutputs());
        return NodeLifter::get_type_alias(node_def->getType());
    }

    string Decompiler::get_node_graph_signature(const mx::NodeGraphPtr& node_graph)
    {
        if (node_graph->hasNodeDefString())
        {
            const mx::NodeDefPtr node_def = node_graph->getNodeDef();
            if (node_def == nullptr)
                throw CompileError{"Cannot find NodeDef for " + node_graph->getName()};
            const string return_type = get_node_def_data_type(node_def);
            const string func_name = node_def->getNodeString();
            const string func_params = inputs_to_parameters(node_def->getActiveInputs());
            return return_type + " " + func_name + "(" + func_params + ")";
        }
        else
        {
            const string return_type = outputs_to_data_type(node_graph->getOutputs());
            const vector<mx::InputPtr> inputs = node_graph->getInputs();
            const string func_params = inputs_to_parameters(inputs);
            const string func_name = get_node_graph_identifier(node_graph);
            if (func_params.empty())
                return return_type + " " + func_name + " => ";
            else
                return return_type + " " + func_name + "(" + func_params + ")";
        }
    }

    string Decompiler::get_node_graph_identifier(const mx::NodeGraphPtr& node_graph)
    {
        const string name = node_graph->getName();
        if (starts_with(name, "NG_"))
            return name.substr(3);
        return name;
    }

    ExprPtr Decompiler::get_node_graph_return_expression(const mx::NodeGraphPtr& node_graph)
    {
        if (node_graph->hasNodeDefString())
        {
            const mx::NodeDefPtr node_def = node_graph->getNodeDef();
            if (node_def == nullptr)
                throw CompileError{"Cannot find NodeDef for " + node_graph->getName()};

            vector<mx::OutputPtr> outputs = node_def->getActiveOutputs();
            vector<ExprPtr> exprs;
            for (mx::OutputPtr& output : outputs)
            {
                mx::OutputPtr ng_out = node_graph->getOutput(output->getName());
                exprs.push_back(port_to_expression(ng_out ? ng_out : output));
            }

            if (exprs.size() == 1)
                return exprs[0];
            return create_expression<UnnamedConstructor>(std::move(exprs));
        }
        else
        {
            const vector<mx::OutputPtr> outputs = node_graph->getOutputs();
            vector<ExprPtr> exprs;
            for (const auto& out : outputs)
                exprs.push_back(port_to_expression(out));

            if (exprs.size() == 1)
                return exprs[0];
            return create_expression<UnnamedConstructor>(std::move(exprs));
        }
    }
}
