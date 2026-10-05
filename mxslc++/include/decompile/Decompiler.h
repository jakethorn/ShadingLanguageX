//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/GraphDecompiler.h"
#include "decompile/SourceCode.h"

namespace mxslc::decompile
{
    // Decompiles a MaterialX document into ShadingLanguageX code. Function definitions (from node defs and node graphs)
    // and variable definitions (from nodes) are written in document order, but always after the code they depend on.
    class Decompiler
    {
    public:
        explicit Decompiler(const fs::path& src_path);
        explicit Decompiler(const string& source);
        // the document is copied, so that the MaterialX libraries of its version can be added to it and its elements can
        // be sorted into the order of code
        explicit Decompiler(const mx::DocumentPtr& document);

        // the graph decompiler of the document refers to this decompiler
        Decompiler(const Decompiler&) = delete;
        Decompiler& operator=(const Decompiler&) = delete;

        string decompile_document();
        string decompile_node(const string& node_name, bool with_dependencies = false);
        string decompile_node(const mx::NodePtr& node, bool with_dependencies = false);
        string decompile_node_def(const string& node_def_name, bool with_dependencies = false);
        string decompile_node_def(const mx::NodeDefPtr& node_def, bool with_dependencies = false);
        string decompile_node_graph(const string& node_graph_name, bool with_dependencies = false);
        string decompile_node_graph(const mx::NodeGraphPtr& node_graph, bool with_dependencies = false);

        const mx::DocumentPtr& document() const { return document_; }

        // true if the node def is defined by the document, i.e., it is not part of a library
        bool is_document_node_def(const mx::NodeDefPtr& node_def) const;
        string get_function_name(const mx::ElementPtr& function) const;
        // true if an argument must be passed for this input, i.e., the parameter was declared without a default
        bool is_required_input(const mx::NodeDefPtr& node_def, const string& input_name) const;
        // true if the variable is assigned to by a function, i.e., it is a nonlocal variable of the function
        bool is_assigned_by_function(const string& variable) const;
        // void functions without outputs are given a placeholder integer output, see Serializer
        bool is_void_function(const mx::NodeDefPtr& node_def) const;

    private:
        // the function of a node def or a node graph
        string decompile_function(const mx::ElementPtr& function, bool with_dependencies);

        void emit_document_attributes();
        void emit_node(const mx::NodePtr& node);
        void emit_function(const mx::ElementPtr& function);
        void emit_dependencies(const vector<mx::ElementPtr>& functions);
        // nonlocal variables without a node, e.g., those with a constant value, must still be declared, the node def is
        // the function that uses it, or null if it is declared for a value that is assigned to it
        void emit_nonlocal_variable(const mx::NodeDefPtr& node_def, const string& name, const string& type_name);
        // each call to a decompile function writes its own code
        void clear_emitted_code();

        SourceCode create_function_definition(const mx::NodeDefPtr& node_def, GraphDecompiler& body);
        SourceCode create_function_definition(const mx::NodeGraphPtr& node_graph, GraphDecompiler& body);
        // the statements of the body of a function, and the value of an output, e.g., its return value
        static vector<SourceCode> create_body_statements(GraphDecompiler& body);
        static SourceCode create_output_value(GraphDecompiler& body, const mx::OutputPtr& output);

        mx::DocumentPtr document_;
        SourceCodeWriter writer_;
        unordered_set<string> function_assigned_variables_;
        unordered_set<mx::NodePtr> emitted_nodes_;
        unordered_set<mx::ElementPtr> emitted_functions_;
        unordered_set<string> emitted_nonlocal_variables_;

        GraphDecompiler graph_decompiler_;
    };
}

#endif //MXSLC_DECOMPILER_H
