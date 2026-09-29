//
// Created by jaket on 29/09/2026.
//

#include "decompile/Code.h"

#include <algorithm>

namespace mxslc::decompile
{
    namespace
    {
        constexpr size_t INDENT_WIDTH = 4;

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

    struct Layout::Node
    {
        Kind kind;
        // the text of text layouts, and the opening bracket of lists
        string text;
        vector<Layout> parts;
        // the closing bracket of lists
        string close;
        size_t width;
    };

    // Writes layouts greedily from the outside in: a list or chain is written on a single line if it fits, together
    // with the code that follows it up to the next place where a line can be broken, otherwise it is broken and its
    // parts are written in the same way.
    class Renderer
    {
    public:
        void write(const Layout& layout, const size_t indent, const size_t trailing)
        {
            const Layout::Node& node = *layout.node_;
            const vector<Layout>& parts = node.parts;
            switch (node.kind)
            {
            case Layout::Kind::Text:
                write_text(node.text);
                break;
            case Layout::Kind::Concat:
                for (size_t i = 0; i < parts.size(); ++i)
                    write(parts[i], indent, rest_width(parts, i + 1, trailing));
                break;
            case Layout::Kind::List:
                if (parts.empty() or fits(layout, trailing))
                {
                    write_flat(layout);
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
            case Layout::Kind::ParameterList:
                if (parts.empty() or fits(layout, trailing))
                {
                    write_flat(layout);
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
            case Layout::Kind::Chain:
                if (fits(layout, trailing))
                {
                    write_flat(layout);
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
            case Layout::Kind::Branch:
                if (fits(layout, trailing))
                {
                    write_flat(layout);
                    break;
                }
                write_text("{");
                new_line(indent + INDENT_WIDTH);
                write(parts.front(), indent + INDENT_WIDTH, 0);
                new_line(indent);
                write_text("}");
                break;
            case Layout::Kind::Lines:
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        new_line(indent);
                    write(parts[i], indent, i + 1 == parts.size() ? trailing : 0);
                }
                break;
            case Layout::Kind::Block:
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
        void write_flat(const Layout& layout)
        {
            const Layout::Node& node = *layout.node_;
            const vector<Layout>& parts = node.parts;
            switch (node.kind)
            {
            case Layout::Kind::Text:
                write_text(node.text);
                break;
            case Layout::Kind::List:
            case Layout::Kind::ParameterList:
                write_text(node.text);
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        write_text(", ");
                    write_flat(parts[i]);
                }
                write_text(node.close);
                break;
            case Layout::Kind::Chain:
                for (size_t i = 0; i < parts.size(); ++i)
                {
                    if (i > 0)
                        write_text(" ");
                    write_flat(parts[i]);
                }
                break;
            case Layout::Kind::Branch:
                write_text("{ ");
                write_flat(parts.front());
                write_text(" }");
                break;
            default:
                for (const Layout& part : parts)
                    write_flat(part);
                break;
            }
        }

        // the length of the parts from the start up to the first place where a line can be broken
        static size_t rest_width(const vector<Layout>& parts, const size_t start, const size_t trailing)
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

        bool fits(const Layout& layout, const size_t trailing) const
        {
            return column_ + layout.width() + trailing <= MAX_LINE_LENGTH;
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

    Layout::Layout(string text) : Layout{Kind::Text, std::move(text), {}}
    {

    }

    Layout::Layout(const char* text) : Layout{string{text}}
    {

    }

    Layout::Layout(const Kind kind, string text, vector<Layout> parts, string close)
    {
        size_t width = text.size() + close.size();
        for (const Layout& part : parts)
            width += part.width();
        if ((kind == Kind::List or kind == Kind::ParameterList) and not parts.empty())
            width += 2 * (parts.size() - 1);
        if (kind == Kind::Chain and not parts.empty())
            width += parts.size() - 1;
        if (kind == Kind::Branch)
            width += 4;

        node_ = std::make_shared<const Node>(Node{kind, std::move(text), std::move(parts), std::move(close), width});
    }

    Layout Layout::concat(vector<Layout> parts)
    {
        return Layout{Kind::Concat, "", std::move(parts)};
    }

    Layout Layout::list(string open, vector<Layout> items, string close)
    {
        return Layout{Kind::List, std::move(open), std::move(items), std::move(close)};
    }

    Layout Layout::parameter_list(vector<Layout> params)
    {
        return Layout{Kind::ParameterList, "(", std::move(params), ")"};
    }

    Layout Layout::chain(vector<Layout> links)
    {
        return Layout{Kind::Chain, "", std::move(links)};
    }

    Layout Layout::branch(Layout value)
    {
        return Layout{Kind::Branch, "", {std::move(value)}};
    }

    Layout Layout::lines(vector<Layout> lines)
    {
        return Layout{Kind::Lines, "", std::move(lines)};
    }

    Layout Layout::block(Layout header, vector<Layout> body)
    {
        body.insert(body.begin(), std::move(header));
        return Layout{Kind::Block, "", std::move(body)};
    }

    vector<Layout> Layout::links() const
    {
        if (node_->kind == Kind::Chain)
            return node_->parts;
        return {*this};
    }

    string Layout::str() const
    {
        Renderer renderer;
        renderer.write(*this, 0, 0);
        return renderer.str();
    }

    size_t Layout::width() const
    {
        return node_->width;
    }

    size_t Layout::head_width() const
    {
        switch (node_->kind)
        {
        case Kind::Concat:
        {
            size_t width = 0;
            for (const Layout& part : node_->parts)
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

    bool Layout::is_breakable() const
    {
        switch (node_->kind)
        {
        case Kind::Text:
            return false;
        case Kind::Concat:
            return std::any_of(node_->parts.begin(), node_->parts.end(), [](const Layout& part) { return part.is_breakable(); });
        case Kind::Chain:
            return node_->parts.size() > 1 or (node_->parts.size() == 1 and node_->parts.front().is_breakable());
        case Kind::List:
        case Kind::ParameterList:
            return not node_->parts.empty();
        default:
            return true;
        }
    }

    Layout Code::operand(const Precedence min_precedence) const
    {
        return precedence < min_precedence ? Layout::concat({"(", layout, ")"}) : layout;
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

            // operators with the same precedence are broken over lines together, e.g., `a` `+ b` `- c`
            const bool is_chained = lhs.precedence == precedence and lhs_precedence == precedence;
            vector<Layout> links = is_chained ? lhs.layout.links() : vector{lhs.operand(lhs_precedence)};
            links.push_back(Layout::concat({op + " ", rhs.operand(next(precedence))}));
            return Code{Layout::chain(std::move(links)), precedence};
        }

        Code unary(const string& op, const Code& operand)
        {
            return Code{Layout::concat({op, operand.operand(Precedence::Compound)}), Precedence::Unary};
        }

        Code absolute(const Code& operand)
        {
            return Code{Layout::concat({"|", operand.operand(next(Precedence::Logical)), "|"}), Precedence::Primary};
        }

        Code member(const Code& value, const string& name)
        {
            return Code{Layout::concat({value.operand(Precedence::Postfix), "." + name}), Precedence::Postfix};
        }

        Code index(const Code& value, const Code& index)
        {
            return Code{Layout::concat({value.operand(Precedence::Postfix), "[", index.layout, "]"}), Precedence::Postfix};
        }

        Code construct(const string& type, const vector<Code>& args)
        {
            vector<Layout> items;
            for (const Code& arg : args)
                items.push_back(arg.layout);
            return Code{Layout::list(type + "{", std::move(items), "}"), Precedence::Primary};
        }

        Code call(const string& function, const string& template_type, const vector<Layout>& args)
        {
            const string template_string = template_type.empty() ? "" : "<" + template_type + ">";
            return Code{Layout::list(function + template_string + "(", args, ")"), Precedence::Primary};
        }

        Code if_expression(const Code& condition, const Code& then_code, const optional<Code>& else_code)
        {
            vector<Layout> links {Layout::concat({"if (", condition.layout, ") ", Layout::branch(then_code.layout)})};

            // e.g., `if (a) { x } else if (b) { y } else { z }`
            if (else_code and else_code->is_if_expression())
            {
                const vector<Layout> else_links = else_code->layout.links();
                links.push_back(Layout::concat({"else ", else_links.front()}));
                links.insert(links.end(), else_links.begin() + 1, else_links.end());
            }
            else if (else_code)
            {
                links.push_back(Layout::concat({"else ", Layout::branch(else_code->layout)}));
            }

            return Code{Layout::chain(std::move(links)), Precedence::Lowest};
        }

        Layout argument(const vector<string>& attributes, const string& name, const Code& value)
        {
            string prefix = join(attributes, " ");
            if (not prefix.empty())
                prefix += " ";
            if (not name.empty())
                prefix += name + " = ";
            return prefix.empty() ? value.layout : Layout::concat({prefix, value.layout});
        }

        Layout with_attributes(const vector<string>& attributes, const Layout& statement)
        {
            if (attributes.empty())
                return statement;

            vector<Layout> lines(attributes.begin(), attributes.end());
            lines.push_back(statement);
            return Layout::lines(std::move(lines));
        }
    }

    void CodeWriter::add(Layout code, const bool is_block)
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
            result += items_[i].code.str();
        }
        return result + "\n";
    }
}
