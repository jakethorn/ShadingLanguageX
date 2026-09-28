//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPHDECOMPILER_H
#define MXSLC_GRAPHDECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "runtime/Argument.h"

namespace mxslc::decompile
{
    class DocumentDecompiler;

    // Decompiles the nodes of a single graph, i.e., the document itself or the node graph that implements a function.
    //
    // Nodes either become statements, e.g., `float x = a + b;`, or are inlined into the expression of the node that
    // uses them. Some nodes are absorbed by the pattern of the node that uses them, e.g., the separate node of a swizzle.
    class GraphDecompiler
    {
    public:
        GraphDecompiler(DocumentDecompiler& document, mx::GraphElementPtr graph, const unordered_set<string>& reserved_names = {});

        const mx::GraphElementPtr& graph() const { return graph_; }
        const vector<mx::NodePtr>& nodes() const { return nodes_; }

        bool is_statement(const mx::NodePtr& node) const;
        size_t use_count(const mx::NodePtr& node) const;

        // statement nodes in the order they should be declared
        vector<mx::NodePtr> ordered_statements() const;
        // statement nodes that must be declared before the statement of this node
        vector<mx::NodePtr> statement_dependencies(const mx::NodePtr& node) const;
        // node defs and node graphs of the document that are used by this node or port
        vector<mx::ElementPtr> function_dependencies(const mx::NodePtr& node) const;
        vector<mx::ElementPtr> function_dependencies(const mx::PortElementPtr& port) const;

        // most nodes create a single statement, but ref arguments are declared as variables before the function call
        void create_statements(const mx::NodePtr& node, vector<StmtPtr>& result);
        ExprPtr create_port_expression(const mx::PortElementPtr& port);

    private:
        struct Use
        {
            mx::ElementPtr consumer;
            string output_name;
        };

        // a run of combine inputs that are all of the outputs of a separate node, e.g., `vec3{uv, 1.0}`
        struct ConstructorRun
        {
            size_t start;
            size_t count;
            mx::NodePtr separate;
        };

        void add_use(const mx::PortElementPtr& port, const mx::ElementPtr& consumer);
        void find_absorbed_nodes();
        void find_statements();
        void create_identifiers(const unordered_set<string>& reserved_names);

        string unique_identifier(const string& name);
        const string& identifier(const mx::NodePtr& node) const;
        string output_identifier(const mx::NodePtr& node, const string& output_name);

        bool is_absorbable_by(const mx::NodePtr& helper, const mx::NodePtr& consumer, size_t expected_uses) const;
        mx::NodePtr connected_node(const mx::InputPtr& input) const;
        mx::NodePtr float_first_convert(const mx::NodePtr& node) const;
        mx::NodePtr swizzle_separate(const mx::NodePtr& node) const;
        vector<ConstructorRun> constructor_runs(const mx::NodePtr& node) const;
        bool is_comparison(const mx::NodePtr& node) const;
        bool is_if_expression(const mx::NodePtr& node) const;
        bool is_geomprop_definition(const mx::NodePtr& node) const;

        ExprPtr create_node_expression(const mx::NodePtr& node);
        ExprPtr create_output_expression(const mx::NodePtr& node, const string& output_name);
        ExprPtr create_input_expression(const mx::NodePtr& node, const string& input_name);
        // an operand is typed if the type of the operation that uses it is known and the operand has the same type
        ExprPtr create_operand_expression(const mx::NodePtr& node, const string& input_name, const mx::NodePtr& operation);
        ExprPtr create_untyped_expression(const mx::NodePtr& node, const string& input_name);
        ExprPtr create_binary_expression(const mx::NodePtr& node, const string& symbol);
        ExprPtr create_comparison_expression(const mx::NodePtr& node);
        ExprPtr create_if_expression(const mx::NodePtr& node);
        ExprPtr create_extract_expression(const mx::NodePtr& node);
        ExprPtr create_combine_expression(const mx::NodePtr& node);
        ExprPtr create_function_call(const mx::NodePtr& node, bool with_out_arguments);
        ExprPtr create_node_graph_reference(const mx::PortElementPtr& port);

        TypePtr node_type(const mx::NodePtr& node) const;
        // the template type needed to select the node def if the return type of the call cannot be inferred
        TypePtr template_type(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const;
        // true if the parameter has the same type in all overloads of the function, i.e., the argument type is known
        bool is_parameter_type_unique(const mx::NodeDefPtr& node_def, const string& param_name) const;
        vector<mx::OutputPtr> return_outputs(const mx::NodePtr& node) const;
        vector<mx::OutputPtr> out_parameter_outputs(const mx::NodePtr& node) const;
        bool is_ref_parameter(const mx::NodeDefPtr& node_def, const string& param_name) const;
        // connections to nodes with a single output do not name the output
        string resolved_output_name(const mx::NodePtr& node, const string& output_name) const;

        void collect_dependencies(const mx::NodePtr& node, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const;
        void collect_dependencies(const mx::PortElementPtr& port, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const;

        DocumentDecompiler& document_;
        mx::GraphElementPtr graph_;
        vector<mx::NodePtr> nodes_;
        unordered_map<mx::NodePtr, vector<Use>> uses_;
        unordered_set<mx::NodePtr> absorbed_;
        unordered_set<mx::NodePtr> statements_;
        unordered_set<string> used_identifiers_;
        unordered_map<mx::NodePtr, string> identifiers_;
        unordered_map<string, string> output_identifiers_;
        // true if the type of the expression being created is known from its context, e.g., `float x = <expr>;`
        bool is_typed_context_{true};
    };
}

#endif //MXSLC_GRAPHDECOMPILER_H
