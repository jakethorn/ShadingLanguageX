//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_HINTRECORDER_H
#define MXSLC_HINTRECORDER_H

#include <limits>
#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc::runtime
{
    class ArgumentList;
}

namespace mxslc::serialize
{
    // Records decompile hints, i.e., attributes that describe the code that created the graph, see decompile_hints.h.
    //
    // While user code is executed, the recorder keeps a stack of frames: the statements being executed, the iterations
    // of for loops and the calls of inline functions. Every node is given the path of the frames it was created in and
    // the values bound to variables are marked on the nodes that hold them. Calls to library functions are recorded, but
    // the frames inside of them are not.
    class HintRecorder
    {
        struct Frame;

    public:
        // the frames at the time a lazy value was created, e.g., a swizzle, which are restored when the value is
        // evaluated, so that its nodes are recorded where the value was created
        using Snapshot = shared_ptr<const vector<Frame>>;

        class RestoredSnapshot
        {
        public:
            RestoredSnapshot(HintRecorder& hints, const Snapshot& snapshot);
            ~RestoredSnapshot();
            RestoredSnapshot(const RestoredSnapshot&) = delete;
            RestoredSnapshot& operator=(const RestoredSnapshot&) = delete;

        private:
            HintRecorder& hints_;
            bool is_restored_{false};
        };

        // hides the frames created by the compiler itself, e.g., the calls to separate and combine of a swizzle
        class HiddenFrame
        {
        public:
            explicit HiddenFrame(HintRecorder& hints);
            ~HiddenFrame();
            HiddenFrame(const HiddenFrame&) = delete;
            HiddenFrame& operator=(const HiddenFrame&) = delete;

        private:
            HintRecorder& hints_;
            bool is_pushed_{false};
        };

        explicit HintRecorder(mx::DocumentPtr doc) : doc_{std::move(doc)} { }

        Snapshot snapshot() const;

        bool is_enabled() const { return is_enabled_; }
        void set_enabled(const bool is_enabled) { is_enabled_ = is_enabled; }
        // records the names of the nodes that hold variables, which can only be known once all code was executed
        void finalise() const;

        void enter_statement(const statements::Statement& stmt);
        void exit_statement(const statements::Statement& stmt);
        void enter_loop_iteration(size_t index);
        void exit_loop_iteration();
        // the values of the arguments are evaluated before the call is entered, indexed by parameter, null for out parameters
        void enter_call(const FuncPtr& func, const runtime::ArgumentList& args, const vector<VarPtr>& arg_values);
        void exit_call();
        // non-inline functions are serialized when they are defined, their body is recorded separately from the caller
        void enter_function_body();
        void exit_function_body();

        void define_function(const FuncPtr& func);
        // marks the node def or node graph of a function with the statement that defined it
        void write_function(const mx::ElementPtr& element) const;
        void define_variable(const VarPtr& var, const string& name);
        void define_parameter(const VarPtr& var, const string& name, const VarPtr& arg_value);
        // a variable whose value was copied from another variable, e.g., `vec2 uv = fragCoord;`, which passes the sources of
        // the other variable on to the inputs it is used by
        void copy_variable(const VarPtr& var, const VarPtr& source);

        void write_node(const mx::NodePtr& node, const runtime::ArgumentList& args, const FuncPtr& func);
        void write_input(const mx::NodePtr& node, const string& input_name, const VarPtr& value);

        // marks the nodes that hold the value of a variable after the current statement
        void bind_variable(const VarPtr& var, const string& name);
        void bind_return_value(const VarPtr& value);
        void bind_out_parameter(const VarPtr& value, const string& param_name);
        // marks a node input that was assigned to by the current statement, e.g., `s.base_color = c;`
        void bind_input(const mx::InputPtr& input);

        // the value as it is written in a skeleton, e.g., "1.0", "{_, 2.0}", or "_" if it is held by nodes
        static string value_skeleton(const VarPtr& var);
        // the expression as it is written in a skeleton, i.e., its code if its value is a compile-time value, e.g.,
        // "foo()", otherwise "_" or the code of its fields that are compile-time values, e.g., "{_, 2.0}"
        static string expression_skeleton(const ExprPtr& expr, const VarPtr& value);
        // the variable that is assigned to by an assignment expression, e.g., q for `q.xy`
        static VarPtr assigned_variable(const ExprPtr& lvalue);

    private:
        // transparent frames are statements that are not recorded, e.g., blocks, but the frames inside them are
        enum class FrameKind { Statement, Loop, Call, Function, Transparent, Hidden };

        struct ArgumentRecord
        {
            string param;
            bool is_named;
            bool is_out;
            // the code of the argument if its value is a compile-time value or it is an out argument, e.g., "out fragColor"
            string code;
        };

        struct Frame
        {
            FrameKind kind;
            size_t id{0};
            // the statement that defined the called function, 0 for library functions
            size_t definition_id{0};
            string function;
            vector<ArgumentRecord> arguments;
            bool is_recorded{false};
        };

        struct Owner
        {
            VarPtr var;
            string name;
            bool is_parameter{false};
        };

        bool is_hidden() const;
        optional<size_t> statement_id() const;
        size_t parent_id() const;
        // the path of the frames, or of the first frames
        string path(size_t frame_count = std::numeric_limits<size_t>::max()) const;
        bool is_created_in_current_frame(const mx::NodePtr& node) const;
        // true if the node was created in the current call or loop iteration, or anywhere if there is none
        bool is_created_in_current_execution(const mx::NodePtr& node) const;
        // statements also bind values that were created earlier, calls only bind the values that they created
        void bind(const VarPtr& var, const string& target, bool is_statement = false);
        // the struct variable and field that a variable is a field of, e.g., "s3.xyz.1"
        optional<string> field_source(const VarPtr& var) const;
        void write_call_records();
        string call_record(const Frame& frame) const;

        mx::DocumentPtr doc_;
        bool is_enabled_{false};
        vector<Frame> frames_;
        vector<vector<Frame>> saved_frames_;
        size_t next_statement_id_{1};
        size_t next_call_id_{1};
        unordered_map<const statements::Statement*, size_t> statement_ids_;
        unordered_set<size_t> recorded_statements_;
        unordered_map<size_t, size_t> statement_parents_;
        unordered_map<const runtime::Function*, size_t> function_ids_;
        unordered_map<const runtime::Variable*, Owner> owners_;
        unordered_map<const runtime::Variable*, VarPtr> sources_;
        // the parameters of calls whose values are used by inputs, e.g., "c4.uv"
        unordered_set<string> used_parameters_;
    };

    // pushes and pops a frame of the hint recorder, even if the statement is exited by an exception, e.g., a return
    class StatementHintFrame
    {
    public:
        StatementHintFrame(HintRecorder& hints, const statements::Statement& stmt);
        ~StatementHintFrame();
        StatementHintFrame(const StatementHintFrame&) = delete;
        StatementHintFrame& operator=(const StatementHintFrame&) = delete;

    private:
        HintRecorder& hints_;
        const statements::Statement& stmt_;
    };
}

#endif //MXSLC_HINTRECORDER_H
