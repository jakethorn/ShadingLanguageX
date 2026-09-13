//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <MaterialXCore/Document.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common.h"
#include "expressions/Expression.h"
#include "statements/Statement.h"

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

        StmtPtr build_node_variable_definition(const mx::NodePtr& node);
        void emit_node(const mx::NodePtr& node, vector<StmtPtr>& target_stmts);

        ExprPtr lift_node(const mx::NodePtr& node);
        ExprPtr port_to_expression(const mx::PortElementPtr& port);
        ExprPtr node_and_output_to_dot_op(const mx::NodePtr& node, const string& output);
        ExprPtr node_graph_name_and_output_to_dot_op(const string& node_graph_name, const string& output);

        void analyze_graph(const mx::ElementPtr& parent);
        bool is_inline_node(const mx::NodePtr& node) const;
        bool is_swizzle_consumed(const mx::NodePtr& node) const;

        string node_to_attributes(const mx::NodePtr& node);
        string node_def_to_attributes(const mx::NodeDefPtr& node_def);
        string outputs_to_data_type(const vector<mx::OutputPtr>& outputs);
        string input_to_argument(const mx::InputPtr& input);
        string inputs_to_arguments(const vector<mx::InputPtr>& inputs);
        string input_to_parameter(const mx::InputPtr& input);
        string inputs_to_parameters(const vector<mx::InputPtr>& inputs);

        string get_node_data_type(const mx::NodePtr& node);
        string get_node_def_data_type(const mx::NodeDefPtr& node_def);
        string get_node_graph_signature(const mx::NodeGraphPtr& node_graph);
        string get_node_graph_identifier(const mx::NodeGraphPtr& node_graph);
        ExprPtr get_node_graph_return_expression(const mx::NodeGraphPtr& node_graph);

        mx::DocumentPtr document_;
        string global_code_;
        vector<StmtPtr> current_statements_;
        bool in_function_{false};

        unordered_set<mx::NodePtr> decompiled_nodes_;
        unordered_set<mx::NodePtr> consumed_nodes_;
        unordered_set<mx::NodePtr> resolving_nodes_;
        unordered_map<mx::NodePtr, size_t> ref_counts_;
        unordered_map<string, string> node_graph_var_names_;
        unordered_set<string> decompiled_node_graphs_;
    };
}

#endif //MXSLC_DECOMPILER_H
