//
// Created by jaket on 29/09/2026.
//

#ifndef MXSLC_DECOMPILE_CODE_H
#define MXSLC_DECOMPILE_CODE_H

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

    // Code that is written on a single line if it fits within the maximum line length, otherwise its lists and chains
    // are broken over multiple lines, starting with the outermost, e.g.,
    //     surfaceshader surface = standard_surface(
    //         base_color = if (x > 0.5) { color3{1.0} }
    //             else { color3{} },
    //         specular_roughness = 0.1
    //     );
    class Layout
    {
    public:
        // text that is never broken, e.g., `float x = `
        Layout(string text = "");
        Layout(const char* text);

        // parts that are written one after another, e.g., `x` `.y`
        static Layout concat(vector<Layout> parts);
        // items separated by commas, e.g., `foo(a, b)`, which are written on their own indented lines if they do not
        // fit, and the list is closed on the line after them
        static Layout list(string open, vector<Layout> items, string close);
        // the parameters of a function, which are written on their own lines without indentation if they do not fit,
        // and the list is closed after the last parameter
        static Layout parameter_list(vector<Layout> params);
        // links that are written on their own indented lines after the first if they do not fit, e.g., `a` `+ b` `+ c`
        // or `if (x) { a }` `else { b }`
        static Layout chain(vector<Layout> links);
        // a branch of an if-expression, e.g., `{ a }`, whose value is written on its own indented line if it does not fit
        static Layout branch(Layout value);
        // lines that are always written separately, e.g., the attributes of a statement and the statement
        static Layout lines(vector<Layout> lines);
        // e.g., `float f(float x)` `{` `return x;` `}`
        static Layout block(Layout header, vector<Layout> body);

        // the links of a chain, or this layout if it is not a chain
        vector<Layout> links() const;

        // the layout broken over lines so that they fit within the maximum line length where possible
        string str() const;

    private:
        enum class Kind { Text, Concat, List, ParameterList, Chain, Branch, Lines, Block };
        struct Node;
        friend class Renderer;

        Layout(Kind kind, string text, vector<Layout> parts, string close = "");

        // the length of the layout written on a single line
        size_t width() const;
        // the length of the layout up to the first place where it can be broken
        size_t head_width() const;
        bool is_breakable() const;

        shared_ptr<const Node> node_;
    };

    // The code of an expression and how tightly it binds, which decides where parentheses are needed when it is the
    // operand of another expression.
    struct Code
    {
        Layout layout;
        Precedence precedence{Precedence::Primary};

        // the code as an operand that must bind at least as tightly as the precedence, e.g., `(a + b)` of `(a + b) * c`
        Layout operand(Precedence min_precedence) const;
        // if-expressions are the only code with the lowest precedence
        bool is_if_expression() const { return precedence == Precedence::Lowest; }
    };

    namespace code
    {
        Code identifier(const string& name);
        // e.g., `1.0`, negative numbers bind like unary expressions, e.g., `-1.0`
        Code literal(const string& text);
        // e.g., `a + b`, `a == b` or `a & b`
        Code binary(const Code& lhs, const string& op, const Code& rhs);
        // e.g., `-a` or `!a`
        Code unary(const string& op, const Code& operand);
        // `|a|`
        Code absolute(const Code& operand);
        // `v.x` or `s.field`
        Code member(const Code& value, const string& name);
        // `v[i]`
        Code index(const Code& value, const Code& index);
        // `vec3{a, b}`
        Code construct(const string& type, const vector<Code>& args);
        // `f<T>(a, b)`, whose arguments are complete, see argument
        Code call(const string& function, const string& template_type, const vector<Layout>& args);
        // `if (c) { a } else { b }`, whose else branch can be implied by the variable it is assigned to, e.g.,
        // `x = if (c) { a };`
        Code if_expression(const Code& condition, const Code& then_code, const optional<Code>& else_code);

        // an argument of a call, e.g., `@uiname "Color" base_color = c`
        Layout argument(const vector<string>& attributes, const string& name, const Code& value);
        // a statement with its attributes on the lines before it
        Layout with_attributes(const vector<string>& attributes, const Layout& statement);
    }

    // Statements and function definitions, which are separated by an empty line if either of them has a body.
    class CodeWriter
    {
    public:
        void add(Layout code, bool is_block = false);
        // keeps only the last statement, e.g., the function that was decompiled without its dependencies
        void keep_last();
        string str() const;

    private:
        struct Item
        {
            Layout code;
            bool is_block;
        };

        vector<Item> items_;
    };
}

#endif //MXSLC_DECOMPILE_CODE_H
