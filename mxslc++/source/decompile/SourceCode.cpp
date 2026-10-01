//
// Created by jaket on 29/09/2026.
//

#include "decompile/SourceCode.h"

#include <algorithm>

namespace mxslc::decompile
{
    namespace
    {
        constexpr size_t INDENT_WIDTH = 4;

        Precedence get_next_precedence(const Precedence precedence)
        {
            return static_cast<Precedence>(static_cast<int>(precedence) + 1);
        }

        Precedence get_binary_precedence(const string& op)
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

    struct SourceCode::Node
    {
        Kind kind;
        // the text of Text nodes, and the opening bracket of lists
        string text;
        vector<SourceCode> parts;
        // the closing bracket of lists
        string close;
        size_t width;
    };

    // Writes source code greedily from the outside in: a list or chain is written on a single line if it fits, together
    // with the code that follows it up to the next place where a line can be broken, otherwise it is broken and its
    // parts are written in the same way.
    class SourceCodeRenderer
    {
    public:
        void write(const SourceCode& code, const size_t indent, const size_t trailing)
        {
            const SourceCode::Node& node = *code.node_;
            const vector<SourceCode>& parts = node.parts;
            switch (node.kind)
            {
            case SourceCode::Kind::Text:
                write_text(node.text);
                break;
            case SourceCode::Kind::Concat:
                for (size_t i = 0; i < parts.size(); ++i)
                    write(parts[i], indent, get_rest_width(parts, i + 1, trailing));
                break;
            case SourceCode::Kind::List:
                if (parts.empty() or fits(code, trailing))
                {
                    write_flat(code);
                    break;
                }
                write_text(node.text);
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    const bool is_last = i + 1 == parts.size();
                    new_line(indent + INDENT_WIDTH);
                    write(parts[i], indent + INDENT_WIDTH, is_last ? 0 : 1);
                    if (not is_last)
                        write_text(",");
                }
                new_line(indent);
                write_text(node.close);
                break;
            case SourceCode::Kind::ParameterList:
                if (parts.empty() or fits(code, trailing))
                {
                    write_flat(code);
                    break;
                }
                write_text(node.text);
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    const bool is_last = i + 1 == parts.size();
                    new_line(indent);
                    write(parts[i], indent, is_last ? node.close.size() + trailing : 1);
                    write_text(is_last ? node.close : ",");
                }
                break;
            case SourceCode::Kind::Chain:
                if (fits(code, trailing))
                {
                    write_flat(code);
                    break;
                }
                // the first link is also indented, so that the lines it is broken over are nested inside the chain
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        new_line(indent + INDENT_WIDTH);
                    write(parts[i], indent + INDENT_WIDTH, i + 1 == parts.size() ? trailing : 0);
                }
                break;
            case SourceCode::Kind::Branch:
                if (fits(code, trailing))
                {
                    write_flat(code);
                    break;
                }
                write_text("{");
                new_line(indent + INDENT_WIDTH);
                write(parts.front(), indent + INDENT_WIDTH, 0);
                new_line(indent);
                write_text("}");
                break;
            case SourceCode::Kind::Lines:
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        new_line(indent);
                    write(parts[i], indent, i + 1 == parts.size() ? trailing : 0);
                }
                break;
            case SourceCode::Kind::Block:
                write(parts.front(), indent, 0);
                new_line(indent);
                write_text("{");
                for (size_t i = 1; i < parts.size(); ++i)
                {
                    new_line(indent + INDENT_WIDTH);
                    write(parts[i], indent + INDENT_WIDTH, 0);
                }
                new_line(indent);
                write_text("}");
                break;
            }
        }

        const string& str() const { return out_; }

    private:
        void write_flat(const SourceCode& code)
        {
            const SourceCode::Node& node = *code.node_;
            const vector<SourceCode>& parts = node.parts;
            switch (node.kind)
            {
            case SourceCode::Kind::Text:
                write_text(node.text);
                break;
            case SourceCode::Kind::List:
            case SourceCode::Kind::ParameterList:
                write_text(node.text);
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        write_text(", ");
                    write_flat(parts[i]);
                }
                write_text(node.close);
                break;
            case SourceCode::Kind::Chain:
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        write_text(" ");
                    write_flat(parts[i]);
                }
                break;
            case SourceCode::Kind::Branch:
                write_text("{ ");
                write_flat(parts.front());
                write_text(" }");
                break;
            default:
                for (const SourceCode& part : parts)
                    write_flat(part);
                break;
            }
        }

        // the length of the parts from the start up to the first place where a line can be broken
        static size_t get_rest_width(const vector<SourceCode>& parts, const size_t start, const size_t trailing)
        {
            size_t width = 0;
            for (size_t i = start; i < parts.size(); ++i)
            {
                if (parts[i].is_breakable())
                    return width + parts[i].head_width();
                width += parts[i].width();
            }
            return width + trailing;
        }

        bool fits(const SourceCode& code, const size_t trailing) const
        {
            return column_ + code.width() + trailing <= MAX_LINE_LENGTH;
        }

        void write_text(const string& text)
        {
            out_ += text;
            column_ += text.size();
        }

        void new_line(const size_t indent)
        {
            out_ += '\n' + string(indent, ' ');
            column_ = indent;
        }

        string out_;
        size_t column_{0};
    };

    SourceCode::SourceCode(string text) : SourceCode{Kind::Text, std::move(text), {}}
    {

    }

    SourceCode::SourceCode(const char* text) : SourceCode{string{text}}
    {

    }

    SourceCode::SourceCode(const Kind kind, string text, vector<SourceCode> parts, string close)
    {
        size_t width = text.size() + close.size();
        for (const SourceCode& part : parts)
            width += part.width();
        if ((kind == Kind::List or kind == Kind::ParameterList) and not parts.empty())
            width += 2 * (parts.size() - 1);
        if (kind == Kind::Chain and not parts.empty())
            width += parts.size() - 1;
        if (kind == Kind::Branch)
            width += 4;

        node_ = std::make_shared<const Node>(Node{kind, std::move(text), std::move(parts), std::move(close), width});
    }

    SourceCode SourceCode::concat(vector<SourceCode> parts)
    {
        return SourceCode{Kind::Concat, "", std::move(parts)};
    }

    SourceCode SourceCode::list(string open, vector<SourceCode> items, string close)
    {
        return SourceCode{Kind::List, std::move(open), std::move(items), std::move(close)};
    }

    SourceCode SourceCode::parameter_list(vector<SourceCode> params)
    {
        return SourceCode{Kind::ParameterList, "(", std::move(params), ")"};
    }

    SourceCode SourceCode::chain(vector<SourceCode> links)
    {
        return SourceCode{Kind::Chain, "", std::move(links)};
    }

    SourceCode SourceCode::branch(SourceCode value)
    {
        return SourceCode{Kind::Branch, "", {std::move(value)}};
    }

    SourceCode SourceCode::lines(vector<SourceCode> lines)
    {
        return SourceCode{Kind::Lines, "", std::move(lines)};
    }

    SourceCode SourceCode::block(SourceCode header, vector<SourceCode> body)
    {
        body.insert(body.begin(), std::move(header));
        return SourceCode{Kind::Block, "", std::move(body)};
    }

    vector<SourceCode> SourceCode::links() const
    {
        if (node_->kind == Kind::Chain)
            return node_->parts;
        return {*this};
    }

    string SourceCode::str() const
    {
        SourceCodeRenderer renderer;
        renderer.write(*this, 0, 0);
        return renderer.str();
    }

    size_t SourceCode::width() const
    {
        return node_->width;
    }

    size_t SourceCode::head_width() const
    {
        switch (node_->kind)
        {
        case Kind::Concat:
        {
            size_t width = 0;
            for (const SourceCode& part : node_->parts)
            {
                if (part.is_breakable())
                    return width + part.head_width();
                width += part.width();
            }
            return width;
        }
        case Kind::List:
        case Kind::ParameterList:
            return node_->parts.empty() ? width() : node_->text.size();
        case Kind::Branch:
            return 1;
        case Kind::Chain:
        case Kind::Lines:
        case Kind::Block:
            return node_->parts.empty() ? 0 : node_->parts.front().head_width();
        default:
            return width();
        }
    }

    bool SourceCode::is_breakable() const
    {
        switch (node_->kind)
        {
        case Kind::Text:
            return false;
        case Kind::Concat:
            return std::any_of(node_->parts.begin(), node_->parts.end(), [](const SourceCode& part) { return part.is_breakable(); });
        case Kind::Chain:
            return node_->parts.size() > 1 or (node_->parts.size() == 1 and node_->parts.front().is_breakable());
        case Kind::List:
        case Kind::ParameterList:
            return not node_->parts.empty();
        default:
            return true;
        }
    }

    SourceCode ExpressionCode::operand(const Precedence min_precedence) const
    {
        return precedence < min_precedence ? SourceCode::concat({"(", code, ")"}) : code;
    }

    ExpressionCode format_identifier(const string& name)
    {
        return ExpressionCode{name, Precedence::Primary};
    }

    ExpressionCode format_literal(const string& text)
    {
        return ExpressionCode{text, not text.empty() and text.front() == '-' ? Precedence::Unary : Precedence::Primary};
    }

    ExpressionCode format_binary_expression(const ExpressionCode& lhs, const string& op, const ExpressionCode& rhs)
    {
        const Precedence precedence = get_binary_precedence(op);
        // relational operators do not chain, `a < b < c` is a ternary relational expression
        const Precedence lhs_precedence = precedence == Precedence::Relational ? get_next_precedence(precedence) : precedence;

        // operators with the same precedence are broken over lines together, e.g., `a` `+ b` `- c`
        const bool is_chained = lhs.precedence == precedence and lhs_precedence == precedence;
        vector<SourceCode> links = is_chained ? lhs.code.links() : vector{lhs.operand(lhs_precedence)};
        links.push_back(SourceCode::concat({op + " ", rhs.operand(get_next_precedence(precedence))}));
        return ExpressionCode{SourceCode::chain(std::move(links)), precedence};
    }

    ExpressionCode format_unary_expression(const string& op, const ExpressionCode& operand)
    {
        return ExpressionCode{SourceCode::concat({op, operand.operand(Precedence::Compound)}), Precedence::Unary};
    }

    ExpressionCode format_absolute_value(const ExpressionCode& operand)
    {
        return ExpressionCode{SourceCode::concat({"|", operand.operand(get_next_precedence(Precedence::Logical)), "|"}), Precedence::Primary};
    }

    ExpressionCode format_member_access(const ExpressionCode& value, const string& name)
    {
        return ExpressionCode{SourceCode::concat({value.operand(Precedence::Postfix), "." + name}), Precedence::Postfix};
    }

    ExpressionCode format_indexing(const ExpressionCode& value, const ExpressionCode& index)
    {
        return ExpressionCode{SourceCode::concat({value.operand(Precedence::Postfix), "[", index.code, "]"}), Precedence::Postfix};
    }

    ExpressionCode format_constructor(const string& type, const vector<ExpressionCode>& args)
    {
        vector<SourceCode> items;
        for (const ExpressionCode& arg : args)
            items.push_back(arg.code);
        return ExpressionCode{SourceCode::list(type + "{", std::move(items), "}"), Precedence::Primary};
    }

    ExpressionCode format_function_call(const string& function, const string& template_type, const vector<SourceCode>& args)
    {
        const string template_string = template_type.empty() ? "" : "<" + template_type + ">";
        return ExpressionCode{SourceCode::list(function + template_string + "(", args, ")"), Precedence::Primary};
    }

    ExpressionCode format_if_expression(const ExpressionCode& condition, const ExpressionCode& then_code, const optional<ExpressionCode>& else_code)
    {
        vector<SourceCode> links {SourceCode::concat({"if (", condition.code, ") ", SourceCode::branch(then_code.code)})};

        // e.g., `if (a) { x } else if (b) { y } else { z }`
        if (else_code and else_code->is_if_expression())
        {
            const vector<SourceCode> else_links = else_code->code.links();
            links.push_back(SourceCode::concat({"else ", else_links.front()}));
            links.insert(links.end(), else_links.begin() + 1, else_links.end());
        }
        else if (else_code)
        {
            links.push_back(SourceCode::concat({"else ", SourceCode::branch(else_code->code)}));
        }

        return ExpressionCode{SourceCode::chain(std::move(links)), Precedence::Lowest};
    }

    SourceCode format_argument(const vector<string>& attributes, const string& name, const ExpressionCode& value)
    {
        string prefix = join(attributes, " ");
        if (not prefix.empty())
            prefix += " ";
        if (not name.empty())
            prefix += name + " = ";
        return prefix.empty() ? value.code : SourceCode::concat({prefix, value.code});
    }

    SourceCode add_attributes(const vector<string>& attributes, const SourceCode& statement)
    {
        if (attributes.empty())
            return statement;

        vector<SourceCode> lines(attributes.begin(), attributes.end());
        lines.push_back(statement);
        return SourceCode::lines(std::move(lines));
    }

    void SourceCodeWriter::add(SourceCode code, const bool is_block)
    {
        items_.push_back(Item{std::move(code), is_block});
    }

    void SourceCodeWriter::keep_last()
    {
        if (items_.size() > 1)
            items_.erase(items_.begin(), items_.end() - 1);
    }

    string SourceCodeWriter::str() const
    {
        if (items_.empty())
            return "";

        string result;
        for (size_t i = 0; i < items_.size(); ++i)
        {
            if (i > 0)
                result += items_[i - 1].is_block or items_[i].is_block ? "\n\n" : "\n";
            result += items_[i].code.str();
        }
        return result + "\n";
    }
}
