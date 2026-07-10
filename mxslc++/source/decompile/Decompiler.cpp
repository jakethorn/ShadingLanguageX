//
// Created by jaket on 19/06/2026.
//


#include <MaterialXFormat/XmlIo.h>

#include "decompile/Decompiler.h"
#include "common.h"
#include "constants.h"
#include "serialize/load_mtlx.h"
#include "serialize/mtlx_utils.h"
#include "utils/io_utils.h"
#include "utils/container_utils.h"
#include "errors/CompileError.h"

namespace mxslc::decompile
{
    using namespace container_utils;

    namespace
    {
        bool is_inline_node(const mx::NodePtr& node)
        {
            return node->getName().rfind("var__", 0) == 0;
        }

        string get_type_alias(const string& type_name)
        {
            static const unordered_map<string, string> type_aliases {
                {"boolean", "bool"},
                {"integer", "int"},
                {"vector2", "vec2"},
                {"vector3", "vec3"},
                {"vector4", "vec4"},
                {"matrix33", "mat3"},
                {"matrix44", "mat4"},
            };

            if (contains(type_aliases, type_name))
                return type_aliases.at(type_name);
            return type_name;
        }

        string get_type_alias(const mx::TypedElementPtr& typed_element)
        {
            return get_type_alias(typed_element->getType());
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

    string Decompiler::decompile_document()
    {
        global_code_ = "";
        decompiled_nodes_.clear();

        for (const mx::NodeGraphPtr& node_graph : document_->getNodeGraphs())
        {
            global_code_ += node_graph_to_function_definition(node_graph);
        }

        for (const mx::NodePtr& node : document_->getNodes())
        {
            if (not is_inline_node(node))
                global_code_ += node_to_variable_definition(node);
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

        string code = node_to_variable_definition(node);
        if (with_dependencies)
            code = global_code_ + code;
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
        if (contains(decompiled_nodes_, node))
            return "";
        decompiled_nodes_.insert(node);

        if (const mx::NodeDefPtr node_def = node->getNodeDef())
            global_code_ += node_def_to_function_definition(node_def);

        const string var_type = get_node_data_type(node);
        const string var_name = node->getName();

        string var_expr = node_to_expression(node);
        if (var_expr.front() == '(' and var_expr.back() == ')')
            var_expr = var_expr.substr(1, var_expr.size() - 2);

        return var_type + " " + var_name + " = " + var_expr + ";\n";
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

        const string signature = get_node_graph_signature(node_graph);

        in_function_ = true;

        function_code_ = "";
        for (const mx::NodePtr& node : node_graph->getNodes())
        {
            if (not is_inline_node(node))
                function_code_ += "\t" + node_to_variable_definition(node);
        }
        function_code_ += "\treturn " + get_node_graph_return_expression(node_graph) + ";";

        in_function_ = false;

        return "\n" + signature + "\n{\n" + function_code_ + "\n}\n";
    }

    string Decompiler::node_to_expression(const mx::NodePtr& node)
    {
        const string func_name = node->getCategory();

        static const unordered_map<string, string> binary_op_names {
            {"add", "+"},
            {"subtract", "-"},
            {"multiply", "*"},
            {"divide", "/"},
            {"modulo", "%"},
            {"power", "^"},
            {"and", "&"},
            {"or", "|"},
            {"xor", "^"}
        };

        if (contains(binary_op_names, func_name))
        {
            const string& op = binary_op_names.at(func_name);
            const vector<mx::InputPtr> inputs = node->getInputs();
            return "(" + port_to_expression(inputs[0]) + " " + op + " " + port_to_expression(inputs[1]) + ")";
        }

        static const unordered_map<string, string> unary_op_names {
            {"not", "!"}
        };

        if (contains(unary_op_names, func_name))
        {
            const string& op = unary_op_names.at(func_name);
            const vector<mx::InputPtr> inputs = node->getInputs();
            return port_to_expression(inputs[0]) + op;
        }

        const string func_args = inputs_to_arguments(node->getInputs());
        return func_name + "(" + func_args + ")";
    }

    string Decompiler::outputs_to_data_type(const vector<mx::OutputPtr>& outputs)
    {
        if (outputs.size() == 1)
        {
            return get_type_alias(outputs[0]->getType());
        }
        else
        {
            string result = "{";
            for (const mx::OutputPtr& output : outputs)
                result += get_type_alias(output) + " " + output->getName() + ", ";
            if (result.size() >= 2)
                result.resize(result.size() - 2);
            return result + "}";
        }
    }

    string Decompiler::port_to_expression(const mx::PortElementPtr& port)
    {
        if (port->hasValue())
            return value_to_constructor(port->getValue());
        if (port->hasInterfaceName())
            return interface_name_to_identifier(port->getInterfaceName());
        if (port->hasNodeName())
        {
            const mx::NodePtr node = port->getConnectedNode();
            if (port->hasOutputString())
                return node_and_output_to_dot_op(node, port->getOutputString());
            return is_inline_node(node) ? node_to_expression(node) : node_to_identifier(node);
        }
        if (port->hasNodeGraphString())
        {
            if (port->hasOutputString())
                return node_graph_name_and_output_to_dot_op(port->getNodeGraphString(), port->getOutputString());
            return node_graph_name_to_identifier(port->getNodeGraphString());
        }
        throw CompileError{"Cannot decompile PortElement: " + port->asString()};
    }

    string Decompiler::outputs_to_expression(const vector<mx::OutputPtr>& outputs)
    {
        if (outputs.size() == 1)
            return port_to_expression(outputs[0]);

        string result = "{";
        for (const mx::OutputPtr& output : outputs)
            result += port_to_expression(output) + ", ";
        if (result.size() >= 2)
            result.resize(result.size() - 2);
        return result + "}";
    }

    string Decompiler::value_to_constructor(const mx::ValuePtr& value)
    {
        const string type_name = get_type_alias(value->getTypeString());
        if (contains(vector{"vec2", "vec3", "vec4", "color3", "color4"}, type_name))
            return type_name + "{" + value->getValueString() + "}";
        return value->getValueString();
    }

    string Decompiler::interface_name_to_identifier(const string& interface_name)
    {
        return interface_name;
    }

    string Decompiler::node_and_output_to_dot_op(const mx::NodePtr& node, const string& output)
    {
        return (is_inline_node(node) ? node_to_expression(node) : node_to_identifier(node)) + "." + output;
    }

    string Decompiler::node_graph_name_and_output_to_dot_op(const string& node_graph_name, const string& output)
    {
        return node_graph_name_to_identifier(node_graph_name) + "." + output;
    }

    string Decompiler::node_to_identifier(const mx::NodePtr& node)
    {
        if (not contains(decompiled_nodes_, node))
        {
            const string code = node_to_variable_definition(node);
            if (in_function_)
                function_code_ += "\t" + code;
            else
                global_code_ += code;
        }
        return node->getName();
    }

    string Decompiler::node_graph_name_to_identifier(const string& node_graph_name)
    {
        if (not contains(decompiled_node_graphs_, node_graph_name))
            global_code_ += node_graph_to_function_definition(node_graph_name);

        if (node_graph_name.rfind("NG_", 0) == 0)
            return node_graph_name.substr(3);
        return node_graph_name;
    }

    string Decompiler::input_to_argument(const mx::InputPtr& input)
    {
        return input->getName() + " = " + port_to_expression(input);
    }

    string Decompiler::inputs_to_arguments(const vector<mx::InputPtr>& inputs)
    {
        string result;
        for (const mx::InputPtr& input : inputs)
            result += input_to_argument(input) + ", ";
        if (result.size() >= 2)
            result.resize(result.size() - 2);
        return result;
    }

    string Decompiler::input_to_parameter(const mx::InputPtr& input)
    {
        string result = input->getType() + " " + input->getName();
        if (input->hasValue())
            result += " = " + value_to_constructor(input->getValue());
        return result;
    }

    string Decompiler::inputs_to_parameters(const vector<mx::InputPtr>& inputs)
    {
        string result;
        for (const mx::InputPtr& input : inputs)
            result += input_to_parameter(input) + ", ";
        if (result.size() >= 2)
            result.resize(result.size() - 2);
        return result;
    }

    string Decompiler::get_node_data_type(const mx::NodePtr& node)
    {
        if (node->isMultiOutputType())
        {
            static const vector<fs::path> include_dirs = io_utils::get_include_directories();
            static const mx::DocumentPtr mtlx_lib = get_materialx_library(DEFAULT_MTLX_VERSION, include_dirs);
            const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node, mtlx_lib);
            return outputs_to_data_type(node_def->getActiveOutputs());
        }
        else
        {
            return get_type_alias(node);
        }
    }

    string Decompiler::get_node_def_data_type(const mx::NodeDefPtr& node_def)
    {
        if (node_def->isMultiOutputType())
            return outputs_to_data_type(node_def->getActiveOutputs());
        else
            return node_def->getType();
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
            return return_type + " " + get_node_graph_identifier(node_graph) + " => ";
        }
    }

    string Decompiler::get_node_graph_identifier(const mx::NodeGraphPtr& node_graph)
    {
        return node_graph_name_to_identifier(node_graph->getName());
    }

    string Decompiler::get_node_graph_return_expression(const mx::NodeGraphPtr& node_graph)
    {
        if (node_graph->hasNodeDefString())
        {
            const mx::NodeDefPtr node_def = node_graph->getNodeDef();
            if (node_def == nullptr)
                throw CompileError{"Cannot find NodeDef for " + node_graph->getName()};
            vector<mx::OutputPtr> outputs = node_def->getActiveOutputs();
            for (mx::OutputPtr& output : outputs)
            {
                const mx::OutputPtr node_graph_output = node_graph->getOutput(output->getName());
                if (node_graph_output != nullptr)
                    output = node_graph_output;
            }
            return outputs_to_expression(outputs);
        }
        else
        {
            return outputs_to_expression(node_graph->getOutputs());
        }
    }
}

