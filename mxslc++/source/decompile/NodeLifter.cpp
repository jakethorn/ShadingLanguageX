//
// Created for MXSL decompiler AST lifting.
//

#include "decompile/NodeLifter.h"

#include "TokenType.h"
#include "expressions/DotOperator.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IfExpression.h"
#include "expressions/IndexingOperator.h"
#include "expressions/Literal.h"
#include "expressions/NamedConstructor.h"
#include "expressions/interface.h"
#include "parse.h"
#include "scan.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace expressions;
    using container_utils::contains;

    string NodeLifter::get_type_alias(const string& type_name)
    {
        static const unordered_map<string, string> type_aliases {
            {"boolean", "bool"},
            {"integer", "int"},
            {"vector2", "vec2"},
            {"vector3", "vec3"},
            {"vector4", "vec4"},
            {"matrix33", "mat3"},
            {"matrix44", "mat4"},
        };

        if (contains(type_aliases, type_name))
            return type_aliases.at(type_name);
        return type_name;
    }

    string NodeLifter::get_type_alias(const mx::TypedElementPtr& typed_element)
    {
        return get_type_alias(typed_element->getType());
    }

    bool NodeLifter::is_structural_attribute(const string& attr_name)
    {
        static const unordered_set<string> structural {
            mx::Element::NAME_ATTRIBUTE,
            mx::TypedElement::TYPE_ATTRIBUTE,
            mx::ValueElement::VALUE_ATTRIBUTE,
            mx::ValueElement::INTERFACE_NAME_ATTRIBUTE,
            mx::PortElement::NODE_NAME_ATTRIBUTE,
            mx::PortElement::NODE_GRAPH_ATTRIBUTE,
            mx::PortElement::OUTPUT_ATTRIBUTE,
            mx::Element::XPOS_ATTRIBUTE,
            mx::Element::YPOS_ATTRIBUTE,
            mx::Backdrop::WIDTH_ATTRIBUTE,
            mx::Backdrop::HEIGHT_ATTRIBUTE,
        };

        if (contains(structural, attr_name))
            return true;

        if (string_utils::starts_with(attr_name, "mxsl:"))
            return true;

        return false;
    }

    AttributeList NodeLifter::extract_attributes(const mx::ElementPtr& element)
    {
        vector<Attribute> attrs;
        for (const string& attr_name : element->getAttributeNames())
        {
            if (!is_structural_attribute(attr_name))
                attrs.emplace_back(attr_name, element->getAttribute(attr_name));
        }
        return AttributeList{std::move(attrs)};
    }

    bool NodeLifter::is_swizzle(
        const mx::NodePtr& node,
        string& out_channels,
        mx::InputPtr& out_source_input
    )
    {
        const string category = node->getCategory();
        if (category != "combine2" && category != "combine3" && category != "combine4")
            return false;

        const size_t count = (category == "combine2") ? 2 : ((category == "combine3") ? 3 : 4);
        string source_node_name;
        string channels;

        for (size_t i = 1; i <= count; ++i)
        {
            const string in_name = "in" + std::to_string(i);
            const mx::InputPtr inp = node->getInput(in_name);
            if (!inp || !inp->hasNodeName() || !inp->hasOutputString())
                return false;

            if (source_node_name.empty())
                source_node_name = inp->getNodeName();
            else if (source_node_name != inp->getNodeName())
                return false;

            const string out_str = inp->getOutputString();
            if (out_str == "outx" || out_str == "outr") channels += 'x';
            else if (out_str == "outy" || out_str == "outg") channels += 'y';
            else if (out_str == "outz" || out_str == "outb") channels += 'z';
            else if (out_str == "outw" || out_str == "outa") channels += 'w';
            else return false;
        }

        const mx::ElementPtr parent = node->getParent();
        if (!parent) return false;

        const mx::NodePtr source_node = parent->getChildOfType<mx::Node>(source_node_name);
        if (!source_node) return false;

        const string src_cat = source_node->getCategory();
        if (src_cat != "separate" && src_cat != "separate2" && src_cat != "separate3" && src_cat != "separate4")
            return false;

        const mx::InputPtr src_in = source_node->getInput("in");
        if (!src_in) return false;

        out_channels = channels;
        out_source_input = src_in;
        return true;
    }

    bool NodeLifter::is_extract_swizzle(
        const mx::NodePtr& node,
        char& out_channel,
        mx::InputPtr& out_source_input
    )
    {
        if (node->getCategory() != "extract")
            return false;

        const mx::InputPtr in_port = node->getInput("in");
        const mx::InputPtr idx_port = node->getInput("index");
        if (!in_port || !idx_port || !idx_port->hasValue())
            return false;

        int idx = 0;
        try {
            idx = std::stoi(idx_port->getValueString());
        } catch (...) {
            return false;
        }

        static const char channel_chars[] = {'x', 'y', 'z', 'w'};
        if (idx < 0 || idx > 3)
            return false;

        out_channel = channel_chars[idx];
        out_source_input = in_port;
        return true;
    }

    bool NodeLifter::is_constructor(
        const mx::NodePtr& node,
        string& out_constructor_name
    )
    {
        const string category = node->getCategory();
        if (category != "combine2" && category != "combine3" && category != "combine4")
            return false;

        string dummy_channels;
        mx::InputPtr dummy_in;
        if (is_swizzle(node, dummy_channels, dummy_in))
            return false;

        const string type_name = get_type_alias(node->getType());
        if (type_name == "vec2" || type_name == "vec3" || type_name == "vec4" ||
            type_name == "color3" || type_name == "color4")
        {
            out_constructor_name = type_name;
            return true;
        }
        return false;
    }

    bool NodeLifter::is_if_expression(const mx::NodePtr& node)
    {
        if (node->getCategory() != "ifequal")
            return false;
        return (node->getInput("value1") && node->getInput("value2") &&
                node->getInput("in1") && node->getInput("in2"));
    }

    bool NodeLifter::is_binary_op(
        const mx::NodePtr& node,
        string& out_dunder_name
    )
    {
        static const unordered_map<string, string> bin_ops {
            {"add", "__add__"},
            {"subtract", "__sub__"},
            {"multiply", "__mul__"},
            {"divide", "__div__"},
            {"modulo", "__mod__"},
            {"power", "__pow__"},
            {"and", "__and__"},
            {"or", "__or__"},
            {"xor", "__pow__"},
        };

        const string cat = node->getCategory();
        if (contains(bin_ops, cat))
        {
            out_dunder_name = bin_ops.at(cat);
            return true;
        }
        return false;
    }

    bool NodeLifter::is_unary_op(
        const mx::NodePtr& node,
        string& out_dunder_name
    )
    {
        if (node->getCategory() == "not")
        {
            out_dunder_name = "__not__";
            return true;
        }
        if (node->getCategory() == "invert")
        {
            if (node->getAttribute("mxsl:op") == "-")
            {
                out_dunder_name = "__neg__";
                return true;
            }
        }
        return false;
    }

    ExprPtr NodeLifter::value_to_expression(const mx::ValuePtr& value)
    {
        Primitive prim{value};
        return create_expression<Literal>(std::move(prim));
    }

    ExprPtr NodeLifter::lift_node(
        const mx::NodePtr& node,
        const PortResolver& port_resolver
    )
    {
        // 0. User inline function call check
        if (node->hasAttribute("mxsl:inline_call"))
        {
            const string call_str = node->getAttribute("mxsl:inline_call");
            vector<Token> tokens = scan_string(call_str);
            Parser parser{std::move(tokens)};
            return parser.expression();
        }

        // 1. Swizzle check
        string swiz_channels;
        mx::InputPtr swiz_in;
        if (is_swizzle(node, swiz_channels, swiz_in))
        {
            ExprPtr val = port_resolver(swiz_in);
            return create_expression<DotOperator>(std::move(val), Token{TokenType::Identifier, std::move(swiz_channels)});
        }

        // 2. Extract swizzle check
        char extract_chan = 0;
        mx::InputPtr extract_in;
        if (is_extract_swizzle(node, extract_chan, extract_in))
        {
            ExprPtr val = port_resolver(extract_in);
            return create_expression<DotOperator>(std::move(val), Token{TokenType::Identifier, string(1, extract_chan)});
        }

        // 3. Named constructor check
        string constructor_name;
        if (is_constructor(node, constructor_name))
        {
            const size_t count = (node->getCategory() == "combine2") ? 2 : ((node->getCategory() == "combine3") ? 3 : 4);
            ArgumentList args;
            const mx::ElementPtr parent = node->getParent();

            for (size_t i = 1; i <= count; ++i)
            {
                const mx::InputPtr inp = node->getInput("in" + std::to_string(i));
                if (!inp) continue;

                // Check if this input and the next input come from outx and outy of a separate2 node
                if (i + 1 <= count && inp->hasNodeName() && (inp->getOutputString() == "outx" || inp->getOutputString() == "outr") && parent)
                {
                    const mx::InputPtr next_inp = node->getInput("in" + std::to_string(i + 1));
                    if (next_inp && next_inp->hasNodeName() && next_inp->getNodeName() == inp->getNodeName() &&
                        (next_inp->getOutputString() == "outy" || next_inp->getOutputString() == "outg"))
                    {
                        if (const mx::NodePtr sep_node = parent->getChildOfType<mx::Node>(inp->getNodeName()))
                        {
                            if ((sep_node->getCategory() == "separate2" || sep_node->getCategory() == "separate") && sep_node->getInput("in"))
                            {
                                args.add(port_resolver(sep_node->getInput("in")));
                                ++i;
                                continue;
                            }
                        }
                    }
                }

                args.add(port_resolver(inp));
            }
            return create_expression<NamedConstructor>(std::move(constructor_name), std::move(args));
        }

        // 4. If expression check
        if (is_if_expression(node))
        {
            const mx::InputPtr val1 = node->getInput("value1");
            const mx::InputPtr val2 = node->getInput("value2");
            const mx::InputPtr in1 = node->getInput("in1");
            const mx::InputPtr in2 = node->getInput("in2");

            ExprPtr cond_expr;
            if (val2->hasValue() && val2->getValueString() == "true")
            {
                cond_expr = port_resolver(val1);
            }
            else
            {
                ArgumentList eq_args{port_resolver(val1), port_resolver(val2)};
                cond_expr = create_expression<FunctionCall>("__eq__", std::move(eq_args));
            }

            ExprPtr then_expr = port_resolver(in1);
            ExprPtr else_expr = port_resolver(in2);
            return create_expression<IfExpression>(std::move(cond_expr), std::move(then_expr), std::move(else_expr));
        }

        // 4b. Relational operators (ifgreater, ifgreatereq)
        if (node->getCategory() == "ifgreater" || node->getCategory() == "ifgreatereq")
        {
            const mx::InputPtr v1 = node->getInput("value1");
            const mx::InputPtr v2 = node->getInput("value2");
            if (v1 && v2)
            {
                const bool is_eq = (node->getCategory() == "ifgreatereq");
                const string op_hint = node->getAttribute("mxsl:op");
                bool is_less = (op_hint == "<" || op_hint == "<=");
                if (!is_less && op_hint.empty())
                {
                    if (v1->hasValue() && !v1->hasNodeName() && v2->hasNodeName())
                        is_less = true;
                }

                if (is_less)
                {
                    string dunder = is_eq ? "__le__" : "__lt__";
                    ArgumentList cmp_args{port_resolver(v2), port_resolver(v1)};
                    return create_expression<FunctionCall>(std::move(dunder), std::move(cmp_args));
                }
                else
                {
                    string dunder = is_eq ? "__ge__" : "__gt__";
                    ArgumentList cmp_args{port_resolver(v1), port_resolver(v2)};
                    return create_expression<FunctionCall>(std::move(dunder), std::move(cmp_args));
                }
            }
        }

        // 5. Binary op check
        string bin_dunder;
        if (is_binary_op(node, bin_dunder))
        {
            const mx::InputPtr in1 = node->getInput("in1");
            const mx::InputPtr in2 = node->getInput("in2");
            if (in1 && in2)
            {
                ExprPtr e1 = port_resolver(in1);
                ExprPtr e2 = port_resolver(in2);

                // Scalar-vector promotion unwrap for subtract / divide / modulo / power
                if (bin_dunder == "__sub__" || bin_dunder == "__div__" || bin_dunder == "__mod__" || bin_dunder == "__pow__")
                {
                    if (in1->hasNodeName())
                    {
                        const mx::ElementPtr parent = node->getParent();
                        if (parent)
                        {
                            if (const mx::NodePtr convert_node = parent->getChildOfType<mx::Node>(in1->getNodeName()))
                            {
                                if (convert_node->getCategory() == "convert" && convert_node->getInput("in"))
                                {
                                    e1 = port_resolver(convert_node->getInput("in"));
                                }
                            }
                        }
                    }
                }

                if (node->getAttribute("mxsl:swapped") == "true")
                    std::swap(e1, e2);

                ArgumentList bin_args{std::move(e1), std::move(e2)};
                return create_expression<FunctionCall>(std::move(bin_dunder), std::move(bin_args));
            }
        }

        // 6. Unary op check
        string un_dunder;
        if (is_unary_op(node, un_dunder))
        {
            const mx::InputPtr in_port = node->getInput("in");
            if (in_port)
            {
                ArgumentList un_args{port_resolver(in_port)};
                return create_expression<FunctionCall>(std::move(un_dunder), std::move(un_args));
            }
        }

        // 7. Variadic min/max folding
        if (node->getCategory() == "min" || node->getCategory() == "max")
        {
            const mx::InputPtr in1 = node->getInput("in1");
            const mx::InputPtr in2 = node->getInput("in2");
            if (in1 && in2)
            {
                ExprPtr e1 = port_resolver(in1);
                ExprPtr e2 = port_resolver(in2);

                vector<Argument> flattened_args;
                if (const auto fc1 = dynamic_cast<FunctionCall*>(e1.get()))
                {
                    if (fc1->name() == node->getCategory())
                    {
                        for (const Argument& a : fc1->arguments())
                            flattened_args.push_back(a);
                    }
                }

                if (flattened_args.empty())
                    flattened_args.emplace_back(std::move(e1), 0);

                flattened_args.emplace_back(std::move(e2), flattened_args.size());
                return create_expression<FunctionCall>(node->getCategory(), ArgumentList{std::move(flattened_args)});
            }
        }

        // 8. Generic FunctionCall
        vector<Argument> args;
        for (const mx::InputPtr& input : node->getInputs())
        {
            if (input->getAttribute("mxsl:member_assign") == "true")
                continue;

            AttributeList input_attrs = extract_attributes(input);
            ExprPtr arg_expr = port_resolver(input);

            string arg_name;
            if (input->getAttribute("mxsl:positional") == "true")
            {
                arg_name = "";
            }
            else if (input->getAttribute("mxsl:named") == "true")
            {
                arg_name = input->getName();
            }
            else
            {
                if (node->getCategory() == "constant" && input->getName() == "value")
                    arg_name = "";
                else
                    arg_name = input->getName();
            }

            args.emplace_back(std::move(input_attrs), ModifierList{}, std::move(arg_name), std::move(arg_expr), args.size());
        }

        return create_expression<FunctionCall>(node->getCategory(), ArgumentList{std::move(args)});
    }
}
