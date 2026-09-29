//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DECOMPILE_HINTS_H
#define MXSLC_DECOMPILE_HINTS_H

#include "common.h"

// Decompile hints are attributes written by the compiler, when the decompile_hints option is enabled, that describe the
// code that created the graph. The decompiler uses them to recreate code that cannot be recovered from the graph alone,
// e.g., inline functions, for loops and constants. All hint attributes are prefixed with "dc_".
//
// Frames: code is executed in frames, which are statements, iterations of for loops and calls to inline functions.
// Statements are identified by an id per statement in the source code, e.g., "s4", iterations by their index, e.g.,
// "l1", and calls by an id per call, e.g., "c12". Frames inside of library functions are not recorded.
//
// Document attributes:
//   dc_s<id>  = "<parent id>|<skeleton>"
//      A statement. The parent is the statement that the statement is part of, e.g., a for loop or a function
//      definition, or 0 for top-level statements. The skeleton is the statement without the parts that are recovered
//      from the graph, which are replaced by "_", e.g., "mutable float x = _;", "x += _;" or
//      "for (int i from {0, 1, 2}) {}". Compile-time values are not part of the graph, so the code of the expressions
//      that created them is part of the skeleton, e.g., "float PI = 3.14159;" or "int x = foo();". Statements that are
//      executed more than once, e.g., in loops and functions, are recorded when they are executed for the first time.
//      Statements of templated functions are recorded once for all of their template types.
//      The characters '%', '<' and '>' of records are encoded as "%25", "%3C" and "%3E".
//   dc_c<id>  = "<function>[@s<definition id>] <argument>..."
//      A call to an inline function. The definition id is the statement that defines the function and is missing for
//      library functions. The arguments are the names of the parameters they were passed to, in the order they were
//      passed, followed by "=" if they were passed by name, e.g., "clamp in low= high=". Out arguments and arguments
//      whose compile-time values are not used by an input are followed by ":" and their code, with '%' and ' ' encoded
//      as "%25" and "%20", e.g., "mainImage@s4 fragColor:out%20fragColor fragCoord".
//
// Node attributes:
//   dc_ctx    = "<frame> <frame>..."
//      The frames the node was created in, from outermost to innermost, e.g., "s4 l1 s7 c40".
//   dc_bind   = "<target>[:<output>]..."
//      The values held by the node. A target is either a variable after a statement, e.g., "s7.x", "s9.ray.origin", or
//      the return value or an out parameter of a call, e.g., "c40", "c40.radius". The output is set for multi-output
//      nodes. Calls only bind the nodes that were created in them. Statements also bind the values they assign that
//      were created earlier in the same call or loop iteration, e.g., `float y = x;` or `p.z = p.x;`.
//   dc_name   = "<name>"
//      The name of the node when it was compiled, if it holds a variable. Nodes renamed since then are decompiled with
//      their new name.
//   dc_args   = "<argument>..."
//      The arguments of a function call, in the same format as the call record. Only written if an argument was named.
//
// Node def and node graph attributes:
//   dc_def    = "s<id>"
//      The statement that defined the function.
//
// Input attributes:
//   dc_src    = "<variable>..."
//      The variables that the value of the input was passed through, from innermost to outermost, e.g., "c41.in1 c40.v"
//      for a value that was passed to the parameter v of call c40 and then to in1 of call c41. The last variable can
//      also be a variable with a compile-time value, e.g., "s3.threshold", where s3 is the statement that assigned the
//      value to the variable, or a field of a struct variable, e.g., "s3.xyz.1", whose value can also be a node output.
//      Copies of variables pass their values on, e.g., "c3.in1 s7.uv c1.fragCoord" for `vec2 uv = fragCoord;`, and a
//      call without a variable is the call's compile-time return value, e.g., "c4".
//   dc_value  = "<value>"
//      The value of the input when it was compiled, if its source is a variable with a compile-time value. The variable
//      is only referenced if the input still has this value, i.e., the value was not edited.
//   dc_bind   = "s<id>"
//      The input was assigned to by a statement, e.g., `s.base_color = c;`.
namespace mxslc::serialize::hints
{
    inline const string PREFIX{"dc_"};
    inline const string STATEMENT_PREFIX{"dc_s"};
    inline const string CALL_PREFIX{"dc_c"};
    inline const string CONTEXT_ATTRIBUTE{"dc_ctx"};
    inline const string BIND_ATTRIBUTE{"dc_bind"};
    inline const string ARGUMENTS_ATTRIBUTE{"dc_args"};
    inline const string NAME_ATTRIBUTE{"dc_name"};
    inline const string SOURCE_ATTRIBUTE{"dc_src"};
    inline const string DEFINITION_ATTRIBUTE{"dc_def"};
    inline const string VALUE_ATTRIBUTE{"dc_value"};
    // replaces the parts of a statement that are recovered from the graph
    inline const string PLACEHOLDER{"_"};

    // the code of an argument in a call record, whose arguments are separated by spaces
    inline string encode_argument(const string& code)
    {
        string result;
        for (const char c : code)
        {
            if (c == '%')
                result += "%25";
            else if (c == ' ')
                result += "%20";
            else
                result += c;
        }
        return result;
    }

    inline string decode_argument(const string& code)
    {
        string result;
        for (size_t i = 0; i < code.size(); ++i)
        {
            if (code[i] == '%' and code.compare(i, 3, "%25") == 0)
                result += '%';
            else if (code[i] == '%' and code.compare(i, 3, "%20") == 0)
                result += ' ';
            else
            {
                result += code[i];
                continue;
            }
            i += 2;
        }
        return result;
    }

    // the XML writer does not escape '<' in attribute values, so records encode it, e.g., in `T foo<vec2, vec3>()`
    inline string encode(const string& value)
    {
        string result;
        for (const char c : value)
        {
            if (c == '%')
                result += "%25";
            else if (c == '<')
                result += "%3C";
            else if (c == '>')
                result += "%3E";
            else
                result += c;
        }
        return result;
    }

    inline string decode(const string& value)
    {
        string result;
        for (size_t i = 0; i < value.size(); ++i)
        {
            const string code = value.substr(i, 3);
            if (code == "%25" or code == "%3C" or code == "%3E")
            {
                result += code == "%25" ? '%' : code == "%3C" ? '<' : '>';
                i += 2;
            }
            else
            {
                result += value[i];
            }
        }
        return result;
    }
}

#endif //MXSLC_DECOMPILE_HINTS_H
