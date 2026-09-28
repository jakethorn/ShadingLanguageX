//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DOCUMENTDECOMPILER_H
#define MXSLC_DOCUMENTDECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/GraphDecompiler.h"
#include "statements/Statement.h"

namespace mxslc::decompile
{
    // Decompiles a document into a list of statements. Function definitions (from node defs and node graphs) and
    // variable definitions (from nodes) are emitted in document order, but always after the statements they depend on.
    class DocumentDecompiler
    {
    public:
        explicit DocumentDecompiler(mx::DocumentPtr document);

        const mx::DocumentPtr& document() const { return document_; }

        string decompile_document();
        string decompile_node(const mx::NodePtr& node, bool with_dependencies);
        string decompile_function(const mx::ElementPtr& function, bool with_dependencies);

        // true if the node def is defined by the document, i.e., it is not part of a library
        bool is_document_node_def(const mx::NodeDefPtr& node_def) const;
        mx::NodeGraphPtr implementation(const mx::NodeDefPtr& node_def) const;
        string function_name(const mx::ElementPtr& function) const;
        // true if an argument must be passed for this input, i.e., the parameter was declared without a default
        bool is_required_input(const mx::NodeDefPtr& node_def, const string& input_name) const;
        // true if the variable is assigned to by a function, i.e., it is a nonlocal variable of the function
        bool is_mutable_variable(const string& name) const;
        // void functions without outputs are given a placeholder integer output, see Serializer
        bool is_void_function(const mx::NodeDefPtr& node_def) const;

    private:
        void emit_document_attributes();
        void emit_node(const mx::NodePtr& node);
        void emit_function(const mx::ElementPtr& function);
        void emit_dependencies(const vector<mx::ElementPtr>& functions);
        // nonlocal variables without a node, e.g., those with a constant value, must still be declared
        void emit_nonlocal_variable(const mx::NodeDefPtr& node_def, const string& name, const string& type_name);

        StmtPtr create_function_definition(const mx::NodeDefPtr& node_def);
        StmtPtr create_function_definition(const mx::NodeGraphPtr& node_graph);
        string print() const;

        mx::DocumentPtr document_;
        GraphDecompiler graph_decompiler_;
        vector<StmtPtr> statements_;
        unordered_set<mx::NodePtr> emitted_nodes_;
        unordered_set<mx::ElementPtr> emitted_functions_;
        unordered_set<string> emitted_nonlocal_variables_;
        unordered_set<string> mutable_variables_;
    };
}

#endif //MXSLC_DOCUMENTDECOMPILER_H
