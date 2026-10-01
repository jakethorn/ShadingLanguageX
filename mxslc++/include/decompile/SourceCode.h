//
// Created by jaket on 29/09/2026.
//

#ifndef MXSLC_DECOMPILE_SOURCE_CODE_H
#define MXSLC_DECOMPILE_SOURCE_CODE_H

#include "common.h"

namespace mxslc::decompile
{
    // lines longer than this are broken over multiple lines where possible
    constexpr size_t MAX_LINE_LENGTH = 100;

    // How tightly an expression binds, matching the order in which the parser handles operators, e.g., Factor binds
    // tighter than Term, so `a + b * c` needs no parentheses.
    enum class Precedence
    {
        // if-expressions
        Lowest,
        Logical,
        Equality,
        Relational,
        Range,
        Term,
        Factor,
        Exponent,
        Unary,
        Compound,
        Increment,
        Postfix,
        Primary
    };

    // Decompiled source code, which is written on a single line if it fits within the maximum line length, otherwise its
    // lists and chains are broken over multiple lines, starting with the outermost, e.g.,
    //     surfaceshader surface = standard_surface(
    //         base_color = if (x > 0.5) { color3{1.0} }
    //             else { color3{} },
    //         specular_roughness = 0.1
    //     );
    class SourceCode
    {
    public:
        // text that is never broken, e.g., `float x = `
        SourceCode(string text = "");
        SourceCode(const char* text);

        // parts that are written one after another, e.g., `x` `.y`
        static SourceCode concat(vector<SourceCode> parts);
        // items separated by commas, e.g., `foo(a, b)`, which are written on their own indented lines if they do not
        // fit, and the list is closed on the line after them
        static SourceCode list(string open, vector<SourceCode> items, string close);
        // the parameters of a function, which are written on their own lines without indentation if they do not fit,
        // and the list is closed after the last parameter
        static SourceCode parameter_list(vector<SourceCode> params);
        // links that are written on their own indented lines after the first if they do not fit, e.g., `a` `+ b` `+ c`
        // or `if (x) { a }` `else { b }`
        static SourceCode chain(vector<SourceCode> links);
        // a branch of an if-expression, e.g., `{ a }`, whose value is written on its own indented line if it does not fit
        static SourceCode branch(SourceCode value);
        // lines that are always written separately, e.g., the attributes of a statement and the statement
        static SourceCode lines(vector<SourceCode> lines);
        // e.g., `float f(float x)` `{` `return x;` `}`
        static SourceCode block(SourceCode header, vector<SourceCode> body);

        // the links of a chain, or this code if it is not a chain
        vector<SourceCode> links() const;

        // the code broken over lines so that they fit within the maximum line length where possible
        string str() const;

    private:
        enum class Kind { Text, Concat, List, ParameterList, Chain, Branch, Lines, Block };
        struct Node;
        friend class SourceCodeRenderer;

        SourceCode(Kind kind, string text, vector<SourceCode> parts, string close = "");

        // the length of the code written on a single line
        size_t width() const;
        // the length of the code up to the first place where it can be broken
        size_t head_width() const;
        bool is_breakable() const;

        shared_ptr<const Node> node_;
    };

    // The source code of an expression and how tightly it binds, which decides where parentheses are needed when it is
    // the operand of another expression.
    struct ExpressionCode
    {
        SourceCode code;
        Precedence precedence{Precedence::Primary};

        // the code as an operand that must bind at least as tightly as the precedence, e.g., `(a + b)` of `(a + b) * c`
        SourceCode operand(Precedence min_precedence) const;
        // if-expressions are the only code with the lowest precedence
        bool is_if_expression() const { return precedence == Precedence::Lowest; }
    };

    ExpressionCode format_identifier(const string& name);
    // e.g., `1.0`, negative numbers bind like unary expressions, e.g., `-1.0`
    ExpressionCode format_literal(const string& text);
    // e.g., `a + b`, `a == b` or `a & b`
    ExpressionCode format_binary_expression(const ExpressionCode& lhs, const string& op, const ExpressionCode& rhs);
    // e.g., `-a` or `!a`
    ExpressionCode format_unary_expression(const string& op, const ExpressionCode& operand);
    // `|a|`
    ExpressionCode format_absolute_value(const ExpressionCode& operand);
    // `v.x` or `s.field`
    ExpressionCode format_member_access(const ExpressionCode& value, const string& name);
    // `v[i]`
    ExpressionCode format_indexing(const ExpressionCode& value, const ExpressionCode& index);
    // `vec3{a, b}`
    ExpressionCode format_constructor(const string& type, const vector<ExpressionCode>& args);
    // `f<T>(a, b)`, whose arguments are complete, see format_argument
    ExpressionCode format_function_call(const string& function, const string& template_type, const vector<SourceCode>& args);
    // `if (c) { a } else { b }`, whose else branch can be implied by the variable it is assigned to, e.g.,
    // `x = if (c) { a };`
    ExpressionCode format_if_expression(const ExpressionCode& condition, const ExpressionCode& then_code, const optional<ExpressionCode>& else_code);

    // an argument of a call, e.g., `@uiname "Color" base_color = c`
    SourceCode format_argument(const vector<string>& attributes, const string& name, const ExpressionCode& value);
    // writes the attributes of a statement on the lines before it
    SourceCode add_attributes(const vector<string>& attributes, const SourceCode& statement);

    // Writes the statements and function definitions of a file, which are separated by an empty line if either of them
    // has a body.
    class SourceCodeWriter
    {
    public:
        void add(SourceCode code, bool is_block = false);
        // keeps only the last statement, e.g., the function that was decompiled without its dependencies
        void keep_last();
        void clear() { items_.clear(); }
        string str() const;

    private:
        struct Item
        {
            SourceCode code;
            bool is_block;
        };

        vector<Item> items_;
    };
}

#endif //MXSLC_DECOMPILE_SOURCE_CODE_H
