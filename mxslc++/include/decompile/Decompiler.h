//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc::decompile
{
    class Decompiler
    {
    public:
        explicit Decompiler(const fs::path& src_path);
        explicit Decompiler(const string& source);
        explicit Decompiler(mx::DocumentPtr document);

        string decompile_document();
        string decompile_node(const string& node_name, bool with_dependencies = false);
        string decompile_node(const mx::NodePtr& node, bool with_dependencies = false);
        string decompile_node_def(const string& node_def_name, bool with_dependencies = false);
        string decompile_node_def(const mx::NodeDefPtr& node_def, bool with_dependencies = false);
        string decompile_node_graph(const string& node_graph_name, bool with_dependencies = false);
        string decompile_node_graph(const mx::NodeGraphPtr& node_graph, bool with_dependencies = false);

    private:
        string node_to_variable_definition(const string& node_name);
        string node_to_variable_definition(const mx::NodePtr& node);
        string node_def_to_function_definition(const string& node_def_name);
        string node_def_to_function_definition(const mx::NodeDefPtr& node_def);
        string node_graph_to_function_definition(const string& node_graph_name);
        string node_graph_to_function_definition(const mx::NodeGraphPtr& node_graph);

        string node_to_expression(const mx::NodePtr& node);
        string outputs_to_data_type(const vector<mx::OutputPtr>& outputs);
        string port_to_expression(const mx::PortElementPtr& port);
        string outputs_to_expression(const vector<mx::OutputPtr>& outputs);
        string value_to_constructor(const mx::ValuePtr& value);
        string interface_name_to_identifier(const string& interface_name);
        string node_and_output_to_dot_op(const mx::NodePtr& node, const string& output);
        string node_graph_name_and_output_to_dot_op(const string& node_graph_name, const string& output);
        string node_to_identifier(const mx::NodePtr& node);
        string node_graph_name_to_identifier(const string& node_graph_name);
        string input_to_argument(const mx::InputPtr& input);
        string inputs_to_arguments(const vector<mx::InputPtr>& inputs);
        string input_to_parameter(const mx::InputPtr& input);
        string inputs_to_parameters(const vector<mx::InputPtr>& inputs);

        string get_node_data_type(const mx::NodePtr& node);
        string get_node_def_data_type(const mx::NodeDefPtr& node_def);
        string get_node_graph_signature(const mx::NodeGraphPtr& node_graph);
        string get_node_graph_identifier(const mx::NodeGraphPtr& node_graph);
        string get_node_graph_return_expression(const mx::NodeGraphPtr& node_graph);

        mx::DocumentPtr document_;
        string global_code_;
        string function_code_;
        bool in_function_{false};
        unordered_set<mx::NodePtr> decompiled_nodes_;
        unordered_set<string> decompiled_node_graphs_;
    };
}

#endif //MXSLC_DECOMPILER_H
