//
// Created by jaket on 29/09/2026.
//

#include "decompile/Code.h"

#include "utils/string_utils.h"

namespace mxslc::decompile
{
    namespace
    {
        Precedence next(const Precedence precedence)
        {
            return static_cast<Precedence>(static_cast<int>(precedence) + 1);
        }

        Precedence binary_precedence(const string& op)
        {
            static const unordered_map<string, Precedence> precedences {
                {"+", Precedence::Term},
                {"-", Precedence::Term},
                {"*", Precedence::Factor},
                {"/", Precedence::Factor},
                {"%", Precedence::Factor},
                {"^", Precedence::Exponent},
                {"==", Precedence::Equality},
                {"!=", Precedence::Equality},
                {">", Precedence::Relational},
                {"<", Precedence::Relational},
                {">=", Precedence::Relational},
                {"<=", Precedence::Relational},
                {"&", Precedence::Logical},
                {"|", Precedence::Logical},
            };
            return precedences.at(op);
        }

        string join(const vector<string>& strings, const string& delimiter)
        {
            string result;
            for (size_t i = 0; i < strings.size(); ++i)
                result += (i > 0 ? delimiter : "") + strings[i];
            return result;
        }
    }

    string Code::operand(const Precedence min_precedence) const
    {
        return precedence < min_precedence ? "(" + text + ")" : text;
    }

    namespace code
    {
        Code identifier(const string& name)
        {
            return Code{name, Precedence::Primary};
        }

        Code literal(const string& text)
        {
            return Code{text, not text.empty() and text.front() == '-' ? Precedence::Unary : Precedence::Primary};
        }

        Code binary(const Code& lhs, const string& op, const Code& rhs)
        {
            const Precedence precedence = binary_precedence(op);
            // relational operators do not chain, `a < b < c` is a ternary relational expression
            const Precedence lhs_precedence = precedence == Precedence::Relational ? next(precedence) : precedence;
            return Code{lhs.operand(lhs_precedence) + " " + op + " " + rhs.operand(next(precedence)), precedence};
        }

        Code unary(const string& op, const Code& operand)
        {
            return Code{op + operand.operand(Precedence::Compound), Precedence::Unary};
        }

        Code absolute(const Code& operand)
        {
            return Code{"|" + operand.operand(next(Precedence::Logical)) + "|", Precedence::Primary};
        }

        Code member(const Code& value, const string& name)
        {
            return Code{value.operand(Precedence::Postfix) + "." + name, Precedence::Postfix};
        }

        Code index(const Code& value, const Code& index)
        {
            return Code{value.operand(Precedence::Postfix) + "[" + index.text + "]", Precedence::Postfix};
        }

        Code construct(const string& type, const vector<Code>& args)
        {
            vector<string> arg_strings;
            for (const Code& arg : args)
                arg_strings.push_back(arg.text);
            return Code{type + "{" + join(arg_strings, ", ") + "}", Precedence::Primary};
        }

        Code call(const string& function, const string& template_type, const vector<string>& args)
        {
            const string template_string = template_type.empty() ? "" : "<" + template_type + ">";
            return Code{function + template_string + "(" + join(args, ", ") + ")", Precedence::Primary};
        }

        Code if_expression(const Code& condition, const Code& then_code, const Code& else_code)
        {
            const string result = "if (" + condition.text + ") { " + then_code.text + " }";

            // e.g., `if (a) { x } else if (b) { y } else { z }`
            const bool is_else_if = else_code.precedence == Precedence::Lowest and string_utils::starts_with(else_code.text, "if (");
            if (is_else_if)
                return Code{result + " else " + else_code.text, Precedence::Lowest};
            return Code{result + " else { " + else_code.text + " }", Precedence::Lowest};
        }

        string argument(const vector<string>& attributes, const string& name, const Code& value)
        {
            string result = join(attributes, " ");
            if (not result.empty())
                result += " ";
            if (not name.empty())
                result += name + " = ";
            return result + value.text;
        }

        string with_attributes(const vector<string>& attributes, const string& statement)
        {
            if (attributes.empty())
                return statement;
            return join(attributes, "\n") + "\n" + statement;
        }

        string block(const string& header, const vector<string>& body)
        {
            if (body.empty())
                return header + "\n{\n}";
            return header + "\n{\n" + string_utils::indent(join(body, "\n")) + "\n}";
        }
    }

    void CodeWriter::add(string code, const bool is_block)
    {
        items_.push_back(Item{std::move(code), is_block});
    }

    void CodeWriter::keep_last()
    {
        if (items_.size() > 1)
            items_.erase(items_.begin(), items_.end() - 1);
    }

    string CodeWriter::str() const
    {
        if (items_.empty())
            return "";

        string result;
        for (size_t i = 0; i < items_.size(); ++i)
        {
            if (i > 0)
                result += items_[i - 1].is_block or items_[i].is_block ? "\n\n" : "\n";
            result += items_[i].code;
        }
        return result + "\n";
    }
}
