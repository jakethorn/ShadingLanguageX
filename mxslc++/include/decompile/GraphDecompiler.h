//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPHDECOMPILER_H
#define MXSLC_GRAPHDECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/Code.h"

namespace mxslc::decompile
{
    class DocumentDecompiler;

    // Decompiles the nodes of a single graph, i.e., the document itself or the node graph that implements a function.
    //
    // Nodes either become statements, e.g., `float x = a + b;`, or are inlined into the expression of the node that
    // uses them. Some nodes are absorbed by the pattern of the node that uses them, e.g., the separate node of a swizzle.
    //
    // The compiler names the values that are assigned to a variable after its definition var__<variable>__<n>, which are
    // written as assignments, e.g., `x = x * 2.0;`, as long as the previous value of the variable is not used after it.
    class GraphDecompiler
    {
    public:
        // the reserved names are the variables that are declared outside of the graph, e.g., the parameters of a function,
        // whose assigned values are written as assignments without a declaration
        GraphDecompiler(DocumentDecompiler& document, mx::GraphElementPtr graph, unordered_set<string> reserved_names = {});

        const vector<mx::NodePtr>& nodes() const { return nodes_; }

        bool is_statement(const mx::NodePtr& node) const;

        // statement nodes in the order they should be declared
        vector<mx::NodePtr> ordered_statements() const;
        // statement nodes that must be declared before the statement of this node
        vector<mx::NodePtr> statement_dependencies(const mx::NodePtr& node) const;
        // node defs and node graphs of the document that are used by this node or port
        vector<mx::ElementPtr> function_dependencies(const mx::NodePtr& node) const;
        vector<mx::ElementPtr> function_dependencies(const mx::PortElementPtr& port) const;

        // most nodes create a single statement, but ref arguments are declared as variables before the function call
        vector<Layout> create_statements(const mx::NodePtr& node);
        optional<Code> create_port_expression(const mx::PortElementPtr& port);

        // the variable that the node is assigned to if the variable is declared outside of the graph, see reserved_names
        optional<string> declared_variable(const mx::NodePtr& node) const;
        // true if values are assigned to the variable in the graph, e.g., `x = x * 2.0;`
        bool is_assigned(const string& variable) const;
        // true if the port is connected to a value that is assigned to the variable, e.g., the output of an out parameter
        bool has_assigned_value(const mx::PortElementPtr& port, const string& variable) const;

    private:
        // the values of a variable in the order that they are assigned, e.g., the nodes named x, var__x__1 and var__x__2
        struct Assignments
        {
            string variable;
            vector<mx::NodePtr> nodes;
            // parameters and nonlocal variables are declared outside of the graph, so all of their values are assignments
            bool is_declared{false};
            string identifier;
        };

        // `q.y = a;` is compiled to `combine3(separate3(q).outx, a, separate3(q).outz)`, which is assigned to q
        struct SwizzleAssignment
        {
            mx::NodePtr previous_separate;
            // the separate node of the value of a swizzle with multiple channels, e.g., the v of `q.xz = v;`
            mx::NodePtr value_separate;
            // the input of the value of a swizzle with a single channel, e.g., the in2 that a is connected to
            string value_input;
            string channels;
        };

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
        void find_assignments();
        void find_absorbed_nodes();
        void find_statements();
        void create_identifiers();
        // removes an assignment that would change the value of a variable while its previous value is still used, and
        // returns true if one was removed, e.g., `x = x + 1.0;` if the previous value of x is used after it
        bool remove_invalid_assignment();
        void remove_assignment(const mx::NodePtr& node);

        size_t use_count(const mx::NodePtr& node) const;
        string unique_identifier(const string& name);
        const string& identifier(const mx::NodePtr& node) const;
        string output_identifier(const mx::NodePtr& node, const string& output_name);

        const Assignments* assignments_of(const mx::NodePtr& node) const;
        // the value of the variable before the node was assigned to it, or null if it is the first value
        mx::NodePtr previous_assignment(const mx::NodePtr& node) const;
        // true if the input has the value of the variable before the node was assigned to it, e.g., the x of `x + 1.0`
        bool is_previous_value(const mx::NodePtr& node, const mx::InputPtr& input) const;
        optional<SwizzleAssignment> find_swizzle_assignment(const mx::NodePtr& node) const;
        // e.g., "+" for `x + a` that is assigned to x, which is written `x += a;`
        optional<string> compound_operator(const mx::NodePtr& node) const;
        // the interface inputs that the expression of the statement uses, e.g., the parameters of the function
        unordered_set<string> interface_dependencies(const mx::PortElementPtr& port) const;
        void collect_interface_dependencies(const mx::PortElementPtr& port, unordered_set<string>& names, unordered_set<mx::NodePtr>& visited) const;

        bool is_absorbable_by(const mx::NodePtr& helper, const mx::NodePtr& consumer, size_t expected_uses) const;
        mx::NodePtr connected_node(const mx::InputPtr& input) const;
        mx::NodePtr float_first_convert(const mx::NodePtr& node) const;
        mx::NodePtr swizzle_separate(const mx::NodePtr& node) const;
        vector<ConstructorRun> constructor_runs(const mx::NodePtr& node) const;
        bool is_comparison(const mx::NodePtr& node) const;
        bool is_if_expression(const mx::NodePtr& node) const;
        bool is_geomprop_definition(const mx::NodePtr& node) const;
        // compile-time values that are returned by a function are written to its outputs as constant nodes
        bool is_output_constant(const mx::NodePtr& node) const;

        optional<Code> create_node_expression(const mx::NodePtr& node);
        optional<Code> create_output_expression(const mx::NodePtr& node, const string& output_name);
        optional<Code> create_input_expression(const mx::NodePtr& node, const string& input_name);
        // an operand is typed if the type of the node that uses it is known and the operand has the same type
        optional<Code> create_operand_expression(const mx::NodePtr& node, const string& input_name);
        optional<Code> create_untyped_expression(const mx::NodePtr& node, const string& input_name);
        optional<Code> create_binary_expression(const mx::NodePtr& node, const string& op);
        optional<Code> create_comparison_expression(const mx::NodePtr& node);
        optional<Code> create_if_expression(const mx::NodePtr& node);
        optional<Code> create_extract_expression(const mx::NodePtr& node);
        optional<Code> create_combine_expression(const mx::NodePtr& node);
        optional<Code> create_function_call(const mx::NodePtr& node, bool with_out_arguments);
        optional<Code> create_node_graph_reference(const mx::PortElementPtr& port);
        // e.g., `mutable float x = a;` for the first value of a variable, `x = x * 2.0;` or `x += b;` for later values
        Layout create_assignment(const mx::NodePtr& node);

        // e.g., `float`, or `{float outx, float outy}` for nodes with multiple outputs
        string node_type(const mx::NodePtr& node) const;
        // the template type needed to select the node def if the return type of the call cannot be inferred
        string template_type(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const;
        // true if the overloads of the function that accept the arguments of the call have different return types
        bool is_return_type_ambiguous(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const;
        // true if the node can be a statement on its own, e.g., `foo();`, otherwise it is assigned to a typed variable
        bool is_untyped_statement(const mx::NodePtr& node) const;
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
        unordered_set<string> reserved_names_;
        unordered_map<mx::NodePtr, vector<Use>> uses_;
        vector<Assignments> assignments_;
        unordered_map<mx::NodePtr, size_t> assignment_indices_;
        unordered_map<mx::NodePtr, SwizzleAssignment> swizzle_assignments_;
        unordered_set<mx::NodePtr> absorbed_;
        unordered_set<mx::NodePtr> statements_;
        unordered_set<string> used_identifiers_;
        unordered_map<mx::NodePtr, string> identifiers_;
        unordered_map<string, string> output_identifiers_;
        // true if the type of the expression being created is known from its context, e.g., `float x = <expr>;`
        bool is_typed_context_{true};
        // the value of the variable that the else branch of an if-expression can omit, e.g., `x = if (c) { a };`
        mx::NodePtr implied_else_;
    };
}

#endif //MXSLC_GRAPHDECOMPILER_H
