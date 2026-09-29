//
// Created by jaket on 28/09/2026.
//

#include "decompile/decompile_utils.h"

#include "Primitive.h"
#include "TokenType.h"
#include "expressions/Identifier.h"
#include "expressions/Literal.h"
#include "expressions/NamedConstructor.h"
#include "expressions/interface.h"
#include "runtime/ArgumentList.h"
#include "runtime/Attribute.h"
#include "runtime/interface.h"
#include "runtime/Type.h"
#include "serialize/decompile_hints.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile_utils
{
    using container_utils::contains;
    using string_utils::starts_with;

    namespace
    {
        // attributes that are expressed by the decompiled code itself or that have no meaning in ShadingLanguageX
        const unordered_set<string>& structural_attributes()
        {
            static const unordered_set<string> attributes {
                mx::Element::NAME_ATTRIBUTE,
                mx::TypedElement::TYPE_ATTRIBUTE,
                mx::ValueElement::VALUE_ATTRIBUTE,
                mx::ValueElement::INTERFACE_NAME_ATTRIBUTE,
                mx::PortElement::NODE_NAME_ATTRIBUTE,
                mx::PortElement::NODE_GRAPH_ATTRIBUTE,
                mx::PortElement::OUTPUT_ATTRIBUTE,
                "channels",
                mx::NodeDef::NODE_ATTRIBUTE,
                mx::Input::DEFAULT_GEOM_PROP_ATTRIBUTE,
                // layout
                mx::Element::XPOS_ATTRIBUTE,
                mx::Element::YPOS_ATTRIBUTE,
                mx::Backdrop::WIDTH_ATTRIBUTE,
                mx::Backdrop::HEIGHT_ATTRIBUTE,
            };
            return attributes;
        }

        const unordered_set<string>& reserved_words()
        {
            // identifiers that are not keywords, but still have a special meaning
            static const unordered_set<string> words {"true", "false", "T", "auto", "void"};
            return words;
        }
    }

    string type_alias(const string& type_name)
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

    TypePtr create_type_from(const string& type_name)
    {
        return create_type(type_alias(type_name));
    }

    bool is_temporary_name(const string& name)
    {
        return starts_with(name, "var__");
    }

    bool is_valid_identifier(const string& name)
    {
        if (name.empty() or std::isdigit(static_cast<unsigned char>(name.front())))
            return false;

        for (const char c : name)
        {
            if (not std::isalnum(static_cast<unsigned char>(c)) and c != '_')
                return false;
        }

        return not TokenType{name}.is_keyword() and not contains(reserved_words(), name);
    }

    string to_identifier(const string& name)
    {
        string result;
        for (const char c : name)
            result += std::isalnum(static_cast<unsigned char>(c)) ? c : '_';

        if (result.empty() or std::isdigit(static_cast<unsigned char>(result.front())))
            result.insert(result.begin(), '_');

        if (not is_valid_identifier(result))
            result += '_';

        return result;
    }

    AttributeList user_attributes(const mx::ElementPtr& element, const string& child_name)
    {
        vector<Attribute> attrs;
        for (const string& attr_name : element->getAttributeNames())
        {
            // ignore namespaced attributes, e.g., xmlns:xi, and decompile hints
            if (contains(structural_attributes(), attr_name) or attr_name.find(':') != string::npos or starts_with(attr_name, serialize::hints::PREFIX))
                continue;
            if (attr_name == mx::InterfaceElement::NODE_DEF_ATTRIBUTE and element->isA<mx::NodeGraph>())
                continue;
            attrs.emplace_back(child_name, attr_name, element->getAttribute(attr_name));
        }
        return AttributeList{std::move(attrs)};
    }

    ExprPtr create_literal(const mx::ValuePtr& value)
    {
        if (value == nullptr)
            return nullptr;

        // zero vectors and colors are written as their default value, e.g., `vec3{}`, and those whose components are all
        // the same as that component, e.g., `color3{1.0}`
        static const unordered_set<string> vector_types {"vector2", "vector3", "vector4", "color3", "color4"};
        if (contains(vector_types, value->getTypeString()))
        {
            if (is_zero(value))
                return create_expression<NamedConstructor>(type_alias(value->getTypeString()), ArgumentList{});

            const vector<string> components = mx::splitString(value->getValueString(), ",");
            const auto is_same = [&](const string& component) { return std::stof(component) == std::stof(components.front()); };
            if (not components.empty() and std::all_of(components.begin(), components.end(), is_same))
            {
                const vector<ExprPtr> args{create_expression<Literal>(Primitive{std::stof(components.front())})};
                return create_expression<NamedConstructor>(type_alias(value->getTypeString()), ArgumentList{args});
            }
        }

        Primitive primitive{value};
        if (primitive.is_null())
            return nullptr;

        return create_expression<Literal>(std::move(primitive));
    }

    ExprPtr create_identifier(const string& name)
    {
        return create_expression<Identifier>(Token{TokenType::Identifier, name});
    }

    Token create_symbol(const string& symbol)
    {
        if (symbol.size() == 1)
            return Token{TokenType{symbol.front()}, symbol};
        return Token{TokenType{symbol}, symbol};
    }

    bool has_literal_syntax(const string& type_name)
    {
        static const unordered_set<string> type_names {
            "boolean", "integer", "float", "vector2", "vector3", "vector4", "color3", "color4", "string", "filename"
        };
        return contains(type_names, type_name);
    }

    vector<string> split_string(const string& str, const string& delimiter)
    {
        vector<string> result;
        size_t start = 0;
        for (size_t end = str.find(delimiter); end != string::npos; end = str.find(delimiter, start))
        {
            result.push_back(str.substr(start, end - start));
            start = end + delimiter.size();
        }
        result.push_back(str.substr(start));
        return result;
    }

    bool is_zero_value(const mx::ValueElementPtr& element)
    {
        return is_zero(element->getValue());
    }

    bool is_zero(const mx::ValuePtr& value)
    {
        if (value == nullptr)
            return false;

        if (value->isA<bool>())
            return not value->asA<bool>();
        if (value->isA<string>())
            return value->asA<string>().empty();

        // numeric values, e.g., "0" or "0, 0, 0"
        const string value_string = value->getValueString();
        size_t start = 0;
        while (start <= value_string.size())
        {
            size_t end = value_string.find(',', start);
            if (end == string::npos)
                end = value_string.size();

            char* parse_end = nullptr;
            const string component = value_string.substr(start, end - start);
            const float f = std::strtof(component.c_str(), &parse_end);
            if (parse_end == component.c_str() or f != 0.0f)
                return false;

            start = end + 1;
        }
        return true;
    }

    optional<char> swizzle_channel(const string& output_name)
    {
        if (output_name.size() == 4 and starts_with(output_name, "out") and string{"xyzwrgba"}.find(output_name[3]) != string::npos)
            return output_name[3];
        return std::nullopt;
    }

    bool is_color_type(const string& type_name)
    {
        return type_name == "color3" or type_name == "color4";
    }

    bool is_connected(const mx::InputPtr& input)
    {
        return input and (not input->getNodeName().empty() or not input->getNodeGraphString().empty() or not input->getInterfaceName().empty());
    }
}
