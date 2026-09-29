//
// Created by jaket on 29/09/2026.
//

#ifndef MXSLC_DECOMPILE_CODE_H
#define MXSLC_DECOMPILE_CODE_H

#include "common.h"

namespace mxslc::decompile
{
    // How tightly an expression binds, matching the order in which the parser handles operators, e.g., Factor binds
    // tighter than Term, so `a + b * c` needs no parentheses.
    enum class Precedence
    {
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

    // The code of an expression and how tightly it binds, which decides where parentheses are needed when it is the
    // operand of another expression.
    struct Code
    {
        string text;
        Precedence precedence{Precedence::Primary};

        // the code as an operand that must bind at least as tightly as the precedence, e.g., `(a + b)` of `(a + b) * c`
        string operand(Precedence min_precedence) const;
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
        Code call(const string& function, const string& template_type, const vector<string>& args);
        // `if (c) { a } else { b }`
        Code if_expression(const Code& condition, const Code& then_code, const Code& else_code);

        // an argument of a call, e.g., `@uiname "Color" base_color = c`
        string argument(const vector<string>& attributes, const string& name, const Code& value);
        // a statement with its attributes on the lines before it
        string with_attributes(const vector<string>& attributes, const string& statement);
    }

    // Statements and function definitions, which are separated by an empty line if either of them has a body.
    class CodeWriter
    {
    public:
        void add(string code, bool is_block = false);
        bool empty() const { return items_.empty(); }
        // keeps only the last statement, e.g., the function that was decompiled without its dependencies
        void keep_last();
        string str() const;

    private:
        struct Item
        {
            string code;
            bool is_block;
        };

        vector<Item> items_;
    };

    namespace code
    {
        // e.g., `float f(float x)\n{\n    return x;\n}`
        string block(const string& header, const vector<string>& body);
    }
}

#endif //MXSLC_DECOMPILE_CODE_H
