//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_HINTEDGRAPH_H
#define MXSLC_HINTEDGRAPH_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/DocumentHints.h"

namespace mxslc::decompile
{
    // The frames of a graph, i.e., the statements, loop iterations and inline function calls that created its nodes,
    // as recorded by the decompile hints.
    //
    // Frames are only trusted if they still match the graph, e.g., after the graph was edited in a node graph editor.
    // Statements that do not match are removed, which leaves their nodes without hints, and calls that do not match are
    // dissolved, which leaves their nodes as part of the calling code.
    class HintedGraph
    {
    public:
        using Frame = DocumentHints::Frame;
        using Binding = DocumentHints::Binding;

        struct Instance
        {
            string key;
            Frame frame;
            Instance* parent{nullptr};
            vector<Instance*> children;
            // the nodes created directly in this frame, i.e., not in one of its children
            vector<mx::NodePtr> nodes;

            bool is_statement() const { return frame.kind == 's'; }
            bool is_loop_iteration() const { return frame.kind == 'l'; }
            bool is_call() const { return frame.kind == 'c'; }
        };

        // a value that is held by a node output
        struct Value
        {
            mx::NodePtr node;
            string output;

            bool operator==(const Value& other) const { return node == other.node and output == other.output; }
        };

        struct BoundValue
        {
            Value value;
            // the field or parameter names of the binding, e.g., {"origin"} for "s9.ray.origin"
            vector<string> path;
        };

        HintedGraph(const DocumentHints& hints, const mx::GraphElementPtr& graph);

        bool empty() const { return root_.children.empty(); }
        const DocumentHints& hints() const { return hints_; }
        const Instance& root() const { return root_; }

        // the innermost frame of a node, or null if the node has no (valid) hints
        const Instance* instance(const mx::NodePtr& node) const;
        // the instance of a call, or null if it did not create nodes or is not valid
        const Instance* call(size_t id) const;
        // a statement that is part of a frame, e.g., a top-level statement of the graph or of the body of a call
        const Instance* statement(const Instance& parent, size_t id) const;

        // removes the hints of the top-level statement that created the node, e.g., if its code cannot be recreated
        void remove_statement(const mx::NodePtr& node);
        // true if a statement was removed, its nodes are decompiled without hints
        bool is_removed(size_t id) const;

        bool is_collapsed(const Instance& call) const;
        void dissolve(const Instance& call);
        // dissolves all calls, or only the calls to library functions
        void dissolve_calls(bool is_library_only);
        bool is_within(const Instance& inner, const Instance& outer) const;
        // the outermost collapsed call that contains the node and is contained by the frame, if any
        const Instance* hiding_call(const mx::NodePtr& node, const Instance* frame) const;

        // the values bound to a statement frame, i.e., the variables it defined or assigned to
        vector<BoundValue> statement_values(const Instance& statement, const string& var_name) const;
        // the values bound to a call, i.e., its return value if the path is empty, otherwise an out parameter
        vector<BoundValue> call_values(const Instance& call) const;
        // the values that a statement assigned that were created before it, e.g., by `float y = x;`, in the frame that the
        // statement is part of, e.g., a call or loop iteration, see HintRecorder::bind
        vector<BoundValue> alias_values(size_t statement_id, const Instance& parent, const string& var_name) const;
        // the inputs that the argument of a call was passed to
        vector<mx::InputPtr> argument_inputs(const Instance& call, const string& param_name) const;

        // all nodes of a frame and its children
        vector<mx::NodePtr> nodes(const Instance& frame) const;

    private:
        Instance* get_or_create(const vector<Frame>& context, size_t length);
        void validate_statements(Instance& parent);
        bool is_valid_statement(const Instance& statement) const;
        void validate_calls(Instance& frame);
        bool is_valid_call(const Instance& call) const;
        void remove(Instance& instance);
        bool is_attached(const Instance& instance) const;
        bool has_external_uses(const Instance& frame, const std::function<bool(const mx::NodePtr&, const string&, const mx::InputPtr&)>& is_allowed) const;

        const DocumentHints& hints_;
        mx::GraphElementPtr graph_;
        vector<mx::NodePtr> nodes_;
        Instance root_;
        unordered_map<string, unique_ptr<Instance>> instances_;
        unordered_map<mx::NodePtr, Instance*> node_instances_;
        unordered_set<const Instance*> dissolved_calls_;
        unordered_set<size_t> removed_statements_;
        // the consumers of each node, i.e., nodes and graph outputs
        struct Use
        {
            mx::ElementPtr consumer;
            string output;
            // the input of the consumer, null if the consumer is a graph output
            mx::InputPtr input;
        };
        unordered_map<mx::NodePtr, vector<Use>> uses_;
    };
}

#endif //MXSLC_HINTEDGRAPH_H
