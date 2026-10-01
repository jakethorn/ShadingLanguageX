//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPHDECOMPILER_H
#define MXSLC_GRAPHDECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/SourceCode.h"

namespace mxslc::decompile
{
    class Decompiler;

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
        // the outside variables are the variables that are declared outside of the graph, e.g., the parameters of a
        // function, whose assigned values are written as assignments without a declaration
        GraphDecompiler(Decompiler& decompiler, mx::GraphElementPtr graph, unordered_set<string> outside_variables = {});

        const vector<mx::NodePtr>& nodes() const { return nodes_; }

        bool is_statement(const mx::NodePtr& node) const;

        // statement nodes in the order they should be declared
        vector<mx::NodePtr> get_ordered_statements() const;
        // statement nodes that must be declared before the statement of this node
        vector<mx::NodePtr> get_statement_dependencies(const mx::NodePtr& node) const;
        // node defs and node graphs of the document that are used by this node or port
        vector<mx::ElementPtr> get_function_dependencies(const mx::NodePtr& node) const;
        vector<mx::ElementPtr> get_function_dependencies(const mx::PortElementPtr& port) const;

        // most nodes create a single statement, but ref arguments are declared as variables before the function call
        vector<SourceCode> create_statements(const mx::NodePtr& node);
        optional<ExpressionCode> create_port_expression(const mx::PortElementPtr& port);

        // the variable that the node is assigned to if the variable is declared outside of the graph, see outside_variables
        optional<string> get_outside_variable(const mx::NodePtr& node) const;
        // true if values are assigned to the variable in the graph, e.g., `x = x * 2.0;`
        bool is_assigned(const string& variable) const;
        // true if the port is connected to a value that is assigned to the variable, e.g., the output of an out parameter
        bool has_assigned_value(const mx::PortElementPtr& port, const string& variable) const;

    private:
        // the values of a variable in the order that they are assigned, e.g., the nodes named x, var__x__1 and var__x__2
        struct VariableAssignments
        {
            string variable;
            vector<mx::NodePtr> nodes;
            // parameters and nonlocal variables are declared outside of the graph, so all of their values are assignments
            bool is_declared_outside{false};
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

        struct NodeUse
        {
            mx::ElementPtr consumer;
            string output_name;
        };

        // an argument of a constructor that is separated into the consecutive inputs of a combine node, e.g., the uv of
        // `vec3{uv, 1.0}`, which is compiled to `combine3(separate2(uv).outx, separate2(uv).outy, 1.0)`
        struct SeparatedArgument
        {
            // the index of the first input of the combine node
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

        size_t get_use_count(const mx::NodePtr& node) const;
        string create_unique_identifier(const string& name);
        const string& get_identifier(const mx::NodePtr& node) const;
        string get_output_identifier(const mx::NodePtr& node, const string& output_name);

        const VariableAssignments* get_assignments(const mx::NodePtr& node) const;
        // the value of the variable before the node was assigned to it, or null if it is the first value
        mx::NodePtr get_previous_value(const mx::NodePtr& node) const;
        // true if the input has the value of the variable before the node was assigned to it, e.g., the x of `x + 1.0`
        bool is_previous_value(const mx::NodePtr& node, const mx::InputPtr& input) const;
        optional<SwizzleAssignment> find_swizzle_assignment(const mx::NodePtr& node) const;
        // e.g., "+" for `x + a` that is assigned to x, which is written `x += a;`
        optional<string> get_compound_operator(const mx::NodePtr& node) const;
        // the interface inputs that the expression of the statement uses, e.g., the parameters of the function
        unordered_set<string> get_interface_dependencies(const mx::PortElementPtr& port) const;
        void collect_interface_dependencies(const mx::PortElementPtr& port, unordered_set<string>& names, unordered_set<mx::NodePtr>& visited) const;

        bool is_absorbable_by(const mx::NodePtr& helper, const mx::NodePtr& consumer, size_t expected_uses) const;
        mx::NodePtr get_connected_node(const mx::InputPtr& input) const;
        mx::NodePtr get_float_operand_convert(const mx::NodePtr& node) const;
        mx::NodePtr get_swizzle_separate(const mx::NodePtr& node) const;
        vector<SeparatedArgument> get_separated_arguments(const mx::NodePtr& node) const;
        bool is_comparison(const mx::NodePtr& node) const;
        bool is_if_expression(const mx::NodePtr& node) const;
        bool is_geomprop_definition(const mx::NodePtr& node) const;
        // compile-time values that are returned by a function are written to its outputs as constant nodes
        bool is_output_constant(const mx::NodePtr& node) const;

        optional<ExpressionCode> create_node_expression(const mx::NodePtr& node);
        optional<ExpressionCode> create_output_expression(const mx::NodePtr& node, const string& output_name);
        optional<ExpressionCode> create_input_expression(const mx::NodePtr& node, const string& input_name);
        // an operand is typed if the type of the node that uses it is known and the operand has the same type
        optional<ExpressionCode> create_operand_expression(const mx::NodePtr& node, const string& input_name);
        optional<ExpressionCode> create_untyped_expression(const mx::NodePtr& node, const string& input_name);
        optional<ExpressionCode> create_binary_expression(const mx::NodePtr& node, const string& op);
        optional<ExpressionCode> create_comparison_expression(const mx::NodePtr& node);
        optional<ExpressionCode> create_if_expression(const mx::NodePtr& node);
        optional<ExpressionCode> create_extract_expression(const mx::NodePtr& node);
        optional<ExpressionCode> create_combine_expression(const mx::NodePtr& node);
        optional<ExpressionCode> create_function_call(const mx::NodePtr& node, bool with_out_arguments);
        optional<ExpressionCode> create_node_graph_reference(const mx::PortElementPtr& port) const;
        // e.g., `mutable float x = a;` for the first value of a variable, `x = x * 2.0;` or `x += b;` for later values
        SourceCode create_assignment(const mx::NodePtr& node);

        // e.g., `float`, or `{float outx, float outy}` for nodes with multiple outputs
        string get_variable_type(const mx::NodePtr& node) const;
        // the template type needed to select the node def if the return type of the call cannot be inferred
        string get_template_type(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const;
        // true if the overloads of the function that accept the arguments of the call have different return types
        bool is_return_type_ambiguous(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const;
        // true if the node can be a statement on its own, e.g., `foo();`, otherwise it is assigned to a typed variable
        bool is_untyped_statement(const mx::NodePtr& node) const;
        // true if the parameter has the same type in all overloads of the function, i.e., the argument type is known
        bool is_parameter_type_unique(const mx::NodeDefPtr& node_def, const string& param_name) const;
        vector<mx::OutputPtr> get_return_outputs(const mx::NodePtr& node) const;
        vector<mx::OutputPtr> get_out_parameter_outputs(const mx::NodePtr& node) const;
        bool is_ref_parameter(const mx::NodeDefPtr& node_def, const string& param_name) const;
        // connections to nodes with a single output do not name the output
        string resolve_output_name(const mx::NodePtr& node, const string& output_name) const;

        void collect_dependencies(const mx::NodePtr& node, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const;
        void collect_dependencies(const mx::PortElementPtr& port, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const;

        Decompiler& decompiler_;
        mx::GraphElementPtr graph_;
        vector<mx::NodePtr> nodes_;
        unordered_set<string> outside_variables_;
        unordered_map<mx::NodePtr, vector<NodeUse>> uses_;
        vector<VariableAssignments> assignments_;
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
