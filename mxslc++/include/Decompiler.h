//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <vector>
#include <unordered_set>
#include <string>
#include <filesystem>

#include <MaterialXCore/Document.h>

namespace mxslc
{
    class Decompiler
    {
    public:
        explicit Decompiler(const std::filesystem::path& src_path);
        explicit Decompiler(const std::string& source);
        explicit Decompiler(MaterialX::DocumentPtr document);

        std::string decompile_document();
        std::string decompile_node(const std::string& node_name, bool with_dependencies = false);
        std::string decompile_node(const MaterialX::NodePtr& node, bool with_dependencies = false);
        std::string decompile_node_def(const std::string& node_def_name, bool with_dependencies = false);
        std::string decompile_node_def(const MaterialX::NodeDefPtr& node_def, bool with_dependencies = false);
        std::string decompile_node_graph(const std::string& node_graph_name, bool with_dependencies = false);
        std::string decompile_node_graph(const MaterialX::NodeGraphPtr& node_graph, bool with_dependencies = false);

    private:
        std::string node_to_variable_definition(const std::string& node_name);
        std::string node_to_variable_definition(const MaterialX::NodePtr& node);
        std::string node_def_to_function_definition(const std::string& node_def_name);
        std::string node_def_to_function_definition(const MaterialX::NodeDefPtr& node_def);
        std::string node_graph_to_function_definition(const std::string& node_graph_name);
        std::string node_graph_to_function_definition(const MaterialX::NodeGraphPtr& node_graph);

        std::string node_to_expression(const MaterialX::NodePtr& node);
        std::string outputs_to_data_type(const std::vector<MaterialX::OutputPtr>& outputs);
        std::string port_to_expression(const MaterialX::PortElementPtr& port);
        std::string outputs_to_expression(const std::vector<MaterialX::OutputPtr>& outputs);
        std::string value_to_constructor(const MaterialX::ValuePtr& value);
        std::string interface_name_to_identifier(const std::string& interface_name);
        std::string node_name_and_output_to_dot_op(const std::string& node_name, const std::string& output);
        std::string node_graph_name_and_output_to_dot_op(const std::string& node_graph_name, const std::string& output);
        std::string node_name_to_identifier(const std::string& node_name);
        std::string node_graph_name_to_identifier(const std::string& node_graph_name);
        std::string input_to_argument(const MaterialX::InputPtr& input);
        std::string inputs_to_arguments(const std::vector<MaterialX::InputPtr>& inputs);
        std::string input_to_parameter(const MaterialX::InputPtr& input);
        std::string inputs_to_parameters(const std::vector<MaterialX::InputPtr>& inputs);

        std::string get_node_data_type(const MaterialX::NodePtr& node);
        std::string get_node_def_data_type(const MaterialX::NodeDefPtr& node_def);
        std::string get_node_graph_signature(const MaterialX::NodeGraphPtr& node_graph);
        std::string get_node_graph_identifier(const MaterialX::NodeGraphPtr& node_graph);
        std::string get_node_graph_return_expression(const MaterialX::NodeGraphPtr& node_graph);

        MaterialX::DocumentPtr document_;
        std::string global_code_;
        std::string function_code_;
        bool in_function_{false};
        std::unordered_set<std::string> decompiled_nodes_;
        std::unordered_set<std::string> decompiled_node_graphs_;
    };
}

#endif //MXSLC_DECOMPILER_H
