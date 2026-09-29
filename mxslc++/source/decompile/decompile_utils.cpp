//
// Created by jaket on 28/09/2026.
//

#include "decompile/decompile_utils.h"

#include "Primitive.h"
#include "TokenType.h"
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

        // the components of a numeric value, e.g., {1, 2, 3} for "1, 2, 3", empty if the value is not numeric
        vector<float> components(const mx::ValuePtr& value)
        {
            vector<float> result;
            for (const string& component : mx::splitString(value->getValueString(), ","))
            {
                const string trimmed = mx::trimSpaces(component);
                char* end = nullptr;
                const float f = std::strtof(trimmed.c_str(), &end);
                if (trimmed.empty() or end != trimmed.c_str() + trimmed.size())
                    return {};
                result.push_back(f);
            }
            return result;
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

    vector<string> user_attributes(const mx::ElementPtr& element, const string& child_name)
    {
        vector<string> result;
        for (const string& attr_name : element->getAttributeNames())
        {
            // ignore namespaced attributes, e.g., xmlns:xi
            if (contains(structural_attributes(), attr_name) or attr_name.find(':') != string::npos)
                continue;
            if (attr_name == mx::InterfaceElement::NODE_DEF_ATTRIBUTE and element->isA<mx::NodeGraph>())
                continue;

            const string name = child_name.empty() ? attr_name : child_name + "." + attr_name;
            result.push_back("@" + name + " \"" + element->getAttribute(attr_name) + "\"");
        }
        return result;
    }

    optional<Code> literal(const mx::ValuePtr& value)
    {
        if (value == nullptr)
            return std::nullopt;

        // zero vectors and colors are written as their default value, e.g., `vec3{}`, and those whose components are all
        // the same as that component, e.g., `color3{1.0}`
        static const unordered_set<string> vector_types {"vector2", "vector3", "vector4", "color3", "color4"};
        if (contains(vector_types, value->getTypeString()))
        {
            const vector<float> values = components(value);
            const bool is_uniform = not values.empty() and std::all_of(values.begin(), values.end(), [&](const float f) { return f == values.front(); });
            if (is_uniform)
            {
                const string type = type_alias(value->getTypeString());
                if (values.front() == 0.0f)
                    return decompile::code::construct(type, {});
                return decompile::code::construct(type, {decompile::code::literal(Primitive{values.front()}.to_string())});
            }
        }

        // matrices have no literal syntax, but the identity matrix is their default value
        const string& type = value->getTypeString();
        if ((type == "matrix33" and value->asA<mx::Matrix33>() == mx::Matrix33::IDENTITY) or (type == "matrix44" and value->asA<mx::Matrix44>() == mx::Matrix44::IDENTITY))
            return Code{"default(" + type_alias(type) + ")"};

        const Primitive primitive{value};
        if (primitive.is_null())
            return std::nullopt;
        return decompile::code::literal(primitive.to_string());
    }

    optional<Code> literal(const mx::ValueElementPtr& element)
    {
        // the defaults of types without literal syntax, e.g., shaders, and of filenames, which would otherwise be strings
        const string& type = element->getType();
        if ((not has_literal_syntax(type) or type == "filename") and element->getValueString().empty())
            return Code{"default(" + type_alias(type) + ")"};
        return literal(element->getValue());
    }

    bool has_literal_syntax(const string& type_name)
    {
        static const unordered_set<string> type_names {
            "boolean", "integer", "float", "vector2", "vector3", "vector4", "color3", "color4", "string", "filename"
        };
        return contains(type_names, type_name);
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
        const vector<float> values = components(value);
        return not values.empty() and std::all_of(values.begin(), values.end(), [](const float f) { return f == 0.0f; });
    }

    bool is_zero_value(const mx::ValueElementPtr& element)
    {
        return is_zero(element->getValue());
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

    bool is_index(const string& field_name)
    {
        return not field_name.empty() and std::all_of(field_name.begin(), field_name.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    }

    bool is_connected(const mx::InputPtr& input)
    {
        return input and (not input->getNodeName().empty() or not input->getNodeGraphString().empty() or not input->getInterfaceName().empty());
    }
}
