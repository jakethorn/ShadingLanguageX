//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DOCUMENTDECOMPILER_H
#define MXSLC_DOCUMENTDECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/DocumentHints.h"
#include "decompile/GraphDecompiler.h"
#include "runtime/Parameter.h"
#include "statements/Statement.h"

namespace mxslc::decompile
{
    // Decompiles a document into a list of statements. Function definitions (from node defs and node graphs) and
    // variable definitions (from nodes) are emitted in document order, but always after the statements they depend on.
    // how much of the decompile hints are used, less of them are used if the code created with them is not correct
    enum class HintUsage
    {
        All,
        // calls to library functions are decompiled from their nodes, e.g., operators
        WithoutLibraryCalls,
        // calls to inline functions are decompiled from their nodes
        WithoutCalls,
        None
    };

    class DocumentDecompiler
    {
    public:
        // hints are only used to decompile the whole document, see DocumentHints
        DocumentDecompiler(mx::DocumentPtr document, HintUsage hint_usage);

        const mx::DocumentPtr& document() const { return document_; }
        const DocumentHints& hints() const { return hints_; }
        HintUsage hint_usage() const { return hint_usage_; }
        // see GraphDecompiler::stale_values
        const unordered_set<string>& stale_values() const { return graph_decompiler_.stale_values(); }
        void set_snapshots(unordered_set<string> values) { graph_decompiler_.set_snapshots(std::move(values)); }

        string decompile_document();
        string decompile_node(const mx::NodePtr& node, bool with_dependencies);
        string decompile_function(const mx::ElementPtr& function, bool with_dependencies);

        // true if the node def is defined by the document, i.e., it is not part of a library
        bool is_document_node_def(const mx::NodeDefPtr& node_def) const;
        mx::NodeGraphPtr implementation(const mx::NodeDefPtr& node_def) const;
        // the name that a function is called by, which is the name of the templated function for its node defs, e.g., foo
        // for the node def foo_float of `T foo<float, vec3>(T v)`
        string function_name(const mx::ElementPtr& function) const;
        // the node defs of the templated function that a node def is part of, if it is decompiled as a templated function
        vector<mx::NodeDefPtr> template_instances(const mx::NodeDefPtr& node_def) const;
        // the template type of a node def of a templated function, e.g., float for foo_float
        TypePtr template_type(const mx::NodeDefPtr& node_def) const;
        // true if an argument must be passed for this input, i.e., the parameter was declared without a default
        bool is_required_input(const mx::NodeDefPtr& node_def, const string& input_name) const;
        // true if the variable is assigned to by a function, i.e., it is a nonlocal variable of the function
        bool is_mutable_variable(const string& name) const;
        // void functions without outputs are given a placeholder integer output, see Serializer
        bool is_void_function(const mx::NodeDefPtr& node_def) const;
        // true if a variable of the document was declared by a statement with hints, or the struct variable of a field that
        // is accessed as a nonlocal variable, e.g., x of x__0
        bool is_hinted_variable(const string& name) const;
        // true if the definition of an inline function was emitted, see DocumentHints
        bool is_inline_function_defined(size_t definition_id) const;

    private:
        void emit_document_attributes();
        // statements with hints are emitted in the order of the code, see DocumentHints
        void emit_hinted_statement(size_t id);
        void emit_node(const mx::NodePtr& node);
        void emit_function(const mx::ElementPtr& function);
        // the functions and nonlocal variables that a function uses, which are declared before the function
        void emit_function_dependencies(const mx::ElementPtr& function);
        // the node defs of a templated function are emitted as the templated function if they have the same body, returns
        // false if they must be emitted as separate functions
        bool emit_template_function(size_t definition_id);
        void emit_dependencies(const vector<mx::ElementPtr>& functions);
        // nonlocal variables without a node, e.g., those with a constant value, must still be declared
        void emit_nonlocal_variable(const mx::NodeDefPtr& node_def, const string& name, const string& type_name);

        StmtPtr create_function_definition(const mx::NodeDefPtr& node_def);
        StmtPtr create_function_definition(const mx::NodeGraphPtr& node_graph);
        // the statements of a function body, except for the nodes that are excluded, e.g., those of a compile-time return
        vector<StmtPtr> create_body_statements(GraphDecompiler& body, const mx::ElementPtr& function, const unordered_set<mx::NodePtr>& excluded);
        // the return statement of a function whose return value is a compile-time value, e.g., `return 1.0;`, which is
        // written to the outputs of its node graph as values or constant nodes, which are added to the constants
        const ReturnStatement* compile_time_return(const mx::ElementPtr& function, const mx::NodeGraphPtr& node_graph, const vector<mx::OutputPtr>& outputs, const GraphDecompiler& body, unordered_set<mx::NodePtr>& constants) const;
        // the names of a template type of a templated function, e.g., vec2 and vector2
        unordered_set<string> template_type_names(size_t definition_id, size_t index) const;
        bool has_hinted_parameters(const mx::ElementPtr& function, const vector<Parameter>& params) const;
        // the node defs or node graphs of a function that was defined by a statement, one per template type
        vector<mx::ElementPtr> function_elements(size_t definition_id) const;
        // the definition of a templated function that is not inline, i.e., whose node defs are created when it is defined
        shared_ptr<FunctionDefinition> template_definition(size_t definition_id) const;
        string print() const;

        enum class TemplateState { Deciding, Merged, Separate };
        struct TemplateFunction
        {
            TemplateState state{TemplateState::Deciding};
            // the node defs or node graphs of the template types, in the order of the template types
            vector<mx::ElementPtr> instances;
        };

        mx::DocumentPtr document_;
        HintUsage hint_usage_;
        DocumentHints hints_;
        vector<StmtPtr> statements_;
        unordered_set<mx::NodePtr> emitted_nodes_;
        unordered_set<mx::ElementPtr> emitted_functions_;
        unordered_set<string> emitted_nonlocal_variables_;
        unordered_set<string> mutable_variables_;
        unordered_set<size_t> inline_functions_;
        std::map<size_t, TemplateFunction> template_functions_;
        // the templated function of the node defs that are emitted as a templated function, keyed by node def name
        unordered_map<string, size_t> template_instances_;
        // declared last, because it uses the other members while it is created, e.g., to name the functions it calls
        GraphDecompiler graph_decompiler_;
    };
}

#endif //MXSLC_DOCUMENTDECOMPILER_H
