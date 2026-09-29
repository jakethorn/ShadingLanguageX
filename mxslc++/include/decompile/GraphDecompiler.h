//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPHDECOMPILER_H
#define MXSLC_GRAPHDECOMPILER_H

#include <MaterialXCore/Document.h>

#include <map>

#include "common.h"
#include "decompile/HintedGraph.h"
#include "runtime/Argument.h"

namespace mxslc::statements
{
    class ExpressionStatement;
    class ForEachLoop;
    class FunctionDefinition;
    class ReturnStatement;
    class MultiVariableDefinition;
    class VariableAssignment;
    class VariableDefinition;
}

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
        GraphDecompiler(DocumentDecompiler& document, mx::GraphElementPtr graph, const unordered_set<string>& reserved_names = {}, bool use_hints = true);

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

        /*
         * decompile hints, see GraphDecompilerHints.cpp
         */

        bool has_hints() const { return hinted_ and (not hinted_->empty() or not port_assignments_.empty()); }
        bool is_hinted(const mx::NodePtr& node) const;
        // the code of a skeleton with the variables it references renamed to their identifiers, e.g., `x2 + 1.0`
        ExprPtr rename_variables(const ExprPtr& expr);
        // true if a variable was declared by a statement with hints
        bool is_hinted_variable(const string& name) const;
        // statements with hints are emitted by statement id, instead of as the nodes they created
        bool has_hinted_statement(size_t id) const;
        void create_hinted_statements(size_t id, vector<StmtPtr>& result);
        // the statements without hints and the functions that must be declared before a statement with hints
        vector<mx::NodePtr> hinted_statement_dependencies(size_t id) const;
        vector<mx::ElementPtr> hinted_function_dependencies(size_t id) const;
        // the definition of an inline function, recreated from its calls in this graph
        StmtPtr create_inline_function(size_t definition_id);
        // values that were referenced after the variable that held them was assigned a new value, e.g., after the graph
        // was edited, which must be copied to another variable before that happens, see snapshots
        const unordered_set<string>& stale_values() const { return stale_values_; }
        void set_snapshots(unordered_set<string> values) { snapshot_values_ = std::move(values); }

    private:
        using Instance = HintedGraph::Instance;

        // a variable that holds a value created by the code of the statements with hints
        struct VariableInfo
        {
            string identifier;
            // the statement that assigned the compile-time value that the variable holds, if it holds one
            optional<size_t> statement;
        };

        // a variable that holds a node output, and the field of the variable, e.g., {"ray", {"origin"}}
        using Holder = std::pair<string, vector<string>>;

        struct HintScope
        {
            // keyed by the name of the variable in the source code
            std::map<string, VariableInfo> variables;
            // all identifiers declared in the scope, including those of variables that were declared again
            unordered_set<string> identifiers;
        };

        // everything that changes while statements are created, so that it can be restored, e.g., to unroll a loop
        struct HintState
        {
            vector<HintScope> scopes{1};
            // the variables that hold node outputs, keyed by "<node name>.<output name>", the most recent holder last, e.g.,
            // both x and y hold the value of x after `float y = x;`
            unordered_map<string, vector<Holder>> value_names;
            // the variables that hold copies of values, which keep them after the variable they were assigned to changes
            unordered_map<string, string> snapshot_names;
            // true while the body of a function is created, whose variables can have the names of the graph's variables
            bool is_function_body{false};
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

        /*
         * decompile hints, see GraphDecompilerHints.cpp
         */

        void find_inline_functions();
        void remove_unnamed_multi_outputs();
        // declares the variables of the out arguments of a node call, e.g., `sincos(x, float s, float c);`
        void declare_out_arguments(const mx::NodePtr& node);
        ExprPtr create_multi_output_expression(const vector<HintedGraph::BoundValue>& values);
        void find_assigned_inputs();
        string print_call_body(const Instance& call, vector<StmtPtr>* body);

        void create_region(const vector<size_t>& ids, const Instance& parent, vector<StmtPtr>& result);
        // values used more than once by the code of a statement are held by a variable, e.g., after calls are dissolved
        void create_shared_values(const Instance& instance, vector<StmtPtr>& result);
        // the statement in a frame, i.e., the parent, whose instance is null if it did not create any nodes
        void create_hinted_statement(size_t id, const Instance* instance, const Instance& parent, vector<StmtPtr>& result);
        // statements that assign values that were created before them, e.g., `float y = x;` or `p.z = p.x;`
        bool create_alias_statement(const DocumentHints::StatementRecord& record, const Instance& parent, vector<StmtPtr>& result);
        bool create_record_statement(const DocumentHints::StatementRecord& record, vector<StmtPtr>& result);
        void create_variable_definition(const VariableDefinition& var_def, const Instance& instance, vector<StmtPtr>& result);
        void create_multi_variable_definition(const MultiVariableDefinition& var_def, const Instance& instance, vector<StmtPtr>& result);
        void create_variable_assignment(const VariableAssignment& assignment, const Instance& instance, vector<StmtPtr>& result);
        void create_port_assignment(const VariableAssignment& assignment, const mx::InputPtr& input, vector<StmtPtr>& result);
        void create_expression_statement(const ExpressionStatement& stmt, const Instance& instance, vector<StmtPtr>& result);
        void create_loop(const ForEachLoop& loop, const Instance& instance, vector<StmtPtr>& result);
        void create_return(const DocumentHints::StatementRecord& record, vector<StmtPtr>& result);
        // `v[i] = x;` is compiled to a call to __set__, which assigns the vector that is the value of the statement
        struct IndexAssignment
        {
            ExprPtr target;
            ExprPtr value;
            mx::InputPtr value_input;
        };
        optional<IndexAssignment> create_index_assignment(const string& identifier, const HintedGraph::Value& value);
        // the body of an inline function that is only called with compile-time values, from its complete skeletons
        bool create_skeleton_body(size_t definition_id, vector<StmtPtr>& result);
        ExprPtr create_swizzle_assignment(const string& var_name, const string& swizzle, const HintedGraph::Value& value);

        ExprPtr create_hinted_output_expression(const mx::NodePtr& node, const string& output_name);
        ExprPtr create_hinted_input_expression(const mx::InputPtr& input, size_t first_source);
        ExprPtr create_call_expression(const Instance& call);
        // an argument of a struct out parameter, e.g., xyz of `foo(xyz)`, whose fields are assigned by the call
        ExprPtr create_struct_out_argument(const vector<HintedGraph::BoundValue>& call_values, const string& param_name);
        // a prefix increment or decrement of a variable, e.g., `++x`
        ExprPtr create_increment_expression(const Instance& call, bool is_increment, const ExprPtr& arg);
        ExprPtr create_value_expression(const HintedGraph::Value& value);
        // the skeleton provides the fields that have compile-time values, e.g., `{_, 1.0}`
        ExprPtr create_struct_expression(const vector<HintedGraph::BoundValue>& values, const TypePtr& type, const ExprPtr& skeleton = nullptr);
        // the roots of a statement, i.e., its nodes and calls that are not used by the statement itself
        vector<std::pair<const Instance*, mx::NodePtr>> statement_roots(const Instance& instance) const;

        VariableInfo* find_variable(const string& name);
        VariableInfo* find_variable_by_identifier(const string& identifier);
        bool is_held_by(const HintedGraph::Value& value, const Holder& holder) const;
        string declare_variable(const string& name, optional<size_t> statement = std::nullopt, const string& preferred_identifier = "");

        void push_scope();
        // returns the values held by the variables of the scope
        vector<HintedGraph::Value> pop_scope();
        void assign_values(const string& identifier, const vector<HintedGraph::BoundValue>& values, bool is_definition);
        ExprPtr variable_expression(const string& identifier, const vector<string>& path) const;
        TypePtr resolve_type(const TypePtr& type) const;

        unique_ptr<HintedGraph> hinted_;
        // the statement whose code is the graph, 0 for the document, or the definition of the function of a node graph
        optional<size_t> root_definition_;
        // the frame whose code is being created, and the call whose body is being created, if any
        const Instance* level_{nullptr};
        const Instance* body_call_{nullptr};
        HintState state_;
        // inputs assigned to after their node was created, e.g., `s.base_color = c;`, are not arguments of the call
        unordered_set<mx::InputPtr> assigned_inputs_;
        std::map<size_t, mx::InputPtr> port_assignments_;
        std::map<size_t, vector<StmtPtr>> inline_function_bodies_;
        unordered_set<string> stale_values_;
        unordered_set<string> snapshot_values_;
        vector<std::pair<string, HintedGraph::Value>> pending_snapshots_;

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
