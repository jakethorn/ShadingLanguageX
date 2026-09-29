//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DOCUMENTHINTS_H
#define MXSLC_DOCUMENTHINTS_H

#include <map>
#include <MaterialXCore/Document.h>

#include "common.h"
#include "Token.h"

namespace mxslc::decompile
{
    // The decompile hints of a document, see serialize/decompile_hints.h. Hints are only trusted as far as they match
    // the graph, so every query returns nothing for hints that are missing or cannot be parsed.
    class DocumentHints
    {
    public:
        // a frame of the context of a node, e.g., "s4" or "l1"
        struct Frame
        {
            char kind;
            size_t id;

            bool operator==(const Frame& other) const { return kind == other.kind and id == other.id; }
        };

        // a value held by a node, e.g., "s7.x", "c40.radius" or "s9.u:outx"
        struct Binding
        {
            Frame frame;
            // the variable name and field names of statement bindings, the field or parameter names of call bindings
            vector<string> path;
            string output;
        };

        // a variable that the value of an input was passed through, e.g., "c41.in1" or "s3.threshold"
        struct Source
        {
            Frame frame;
            string name;
        };

        struct Argument
        {
            string param;
            bool is_named;
            // the code of an out argument or of a compile-time value that is not used by an input, e.g., "out fragColor"
            optional<string> code;
        };

        struct StatementRecord
        {
            size_t id;
            size_t parent;
            string skeleton;
            // the parsed skeleton, null if it could not be parsed
            shared_ptr<statements::Statement> statement;
            // true if the skeleton has no placeholders, i.e., the statement can be created without the graph
            bool is_complete;
        };

        struct CallRecord
        {
            size_t id;
            string function;
            // the statement that defines the function, 0 for library functions
            size_t definition;
            vector<Argument> arguments;
        };

        DocumentHints() = default;
        explicit DocumentHints(const mx::DocumentPtr& doc);

        bool empty() const { return statements_.empty() and calls_.empty(); }

        const StatementRecord* statement(size_t id) const;
        const CallRecord* call(size_t id) const;
        // the statements that are part of a statement, e.g., the body of a loop or function, 0 for top-level statements
        vector<size_t> children(size_t parent) const;
        // true if the statement defines an inline function, which excludes comptime functions, whose code is evaluated when
        // it is compiled, like with the reduce_graph option, so they are not decompiled
        bool is_inline_function(size_t definition_id) const;

        static vector<Frame> context(const mx::ElementPtr& element);
        static vector<Binding> bindings(const mx::ElementPtr& element);
        static vector<Source> sources(const mx::ElementPtr& element);
        static optional<vector<Argument>> arguments(const mx::ElementPtr& element);
        // the statement that defined a node def or node graph
        static optional<size_t> definition(const mx::ElementPtr& element);
        // the statement that assigned to an input, e.g., `s.base_color = c;`
        static optional<size_t> assignment(const mx::ElementPtr& input);

        // true if the input still has the value it was compiled with, see hints::VALUE_ATTRIBUTE
        static bool is_unedited(const mx::InputPtr& input);
        // the tokens of a skeleton or of the code of a skeleton, which can be parsed
        static vector<Token> scan_skeleton(const string& skeleton);

        // true if the attribute is a decompile hint
        static bool is_hint(const string& attribute_name);

    private:
        std::map<size_t, StatementRecord> statements_;
        std::map<size_t, CallRecord> calls_;
    };
}

#endif //MXSLC_DOCUMENTHINTS_H
