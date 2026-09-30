//
// Created by jaket on 28/09/2026.
//

#include "decompile/decompile_utils.h"

#include "Primitive.h"
#include "TokenType.h"
#include "serialize/name_prefix_utils.h"
#include "utils/container_utils.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile_utils
{
    using container_utils::contains;
    using string_utils::starts_with;

    namespace
    {
        const unordered_set<string>& get_structural_attributes()
        {
            static const unordered_set<string> attributes {
                mx::Element::NAME_ATTRIBUTE,
                mx::TypedElement::TYPE_ATTRIBUTE,
                mx::ValueElement::VALUE_ATTRIBUTE,
                mx::ValueElement::INTERFACE_NAME_ATTRIBUTE,
                mx::PortElement::NODE_NAME_ATTRIBUTE,
                mx::PortElement::NODE_GRAPH_ATTRIBUTE,
                mx::PortElement::OUTPUT_ATTRIBUTE,
                mx::NodeDef::NODE_ATTRIBUTE,
                mx::Input::DEFAULT_GEOM_PROP_ATTRIBUTE,
                mx::Element::XPOS_ATTRIBUTE,
                mx::Element::YPOS_ATTRIBUTE,
                mx::Backdrop::WIDTH_ATTRIBUTE,
                mx::Backdrop::HEIGHT_ATTRIBUTE,
            };
            return attributes;
        }

        struct TypeInfo
        {
            // the ShadingLanguageX name of the type, e.g., vec3 for vector3
            string alias;
            // e.g., `1.0`, `"text"` or `vec3{1.0, 2.0, 3.0}`, but not matrices or shaders
            bool has_literal_syntax;
            // the channels of vectors and colors, which are accessed by swizzles, e.g., `v.x` or `c.r`
            string channels;
        };

        // the MaterialX types that can be used in ShadingLanguageX
        const unordered_map<string, TypeInfo>& get_type_infos()
        {
            static const unordered_map<string, TypeInfo> types {
                {"boolean", {"bool", true, ""}},
                {"integer", {"int", true, ""}},
                {"float", {"float", true, ""}},
                {"vector2", {"vec2", true, "xy"}},
                {"vector3", {"vec3", true, "xyz"}},
                {"vector4", {"vec4", true, "xyzw"}},
                {"color3", {"color3", true, "rgb"}},
                {"color4", {"color4", true, "rgba"}},
                {"matrix33", {"mat3", false, ""}},
                {"matrix44", {"mat4", false, ""}},
                {"string", {"string", true, ""}},
                {"filename", {"filename", true, ""}},
                {"surfaceshader", {"surfaceshader", false, ""}},
                {"displacementshader", {"displacementshader", false, ""}},
                {"volumeshader", {"volumeshader", false, ""}},
                {"lightshader", {"lightshader", false, ""}},
                {"material", {"material", false, ""}},
                {"BSDF", {"BSDF", false, ""}},
                {"EDF", {"EDF", false, ""}},
                {"VDF", {"VDF", false, ""}},
            };
            return types;
        }

        const unordered_set<string>& get_reserved_words()
        {
            // identifiers that are not keywords, but still have a special meaning
            static const unordered_set<string> words {"true", "false", "T", "auto", "void"};
            return words;
        }
    }

    string get_type_alias(const string& type_name)
    {
        if (contains(get_type_infos(), type_name))
            return get_type_infos().at(type_name).alias;
        return type_name;
    }

    string get_type_alias(const mx::TypedElementPtr& elem)
    {
        return get_type_alias(elem->getType());
    }

    bool is_type_name(const string& name)
    {
        return contains(get_type_infos(), name);
    }

    bool has_literal_syntax(const string& type_name)
    {
        return contains(get_type_infos(), type_name) and get_type_infos().at(type_name).has_literal_syntax;
    }

    string get_swizzle_channels(const string& type_name)
    {
        if (contains(get_type_infos(), type_name))
            return get_type_infos().at(type_name).channels;
        return "";
    }

    bool is_color_type(const string& type_name)
    {
        const string channels = get_swizzle_channels(type_name);
        return not channels.empty() and channels.front() == 'r';
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

        return not TokenType{name}.is_keyword() and not contains(get_reserved_words(), name);
    }

    string make_identifier(const string& name)
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

    vector<string> get_user_attributes(const mx::ElementPtr& element, const string& child_name)
    {
        vector<string> result;
        for (const string& attr_name : element->getAttributeNames())
        {
            // ignore namespaced attributes, e.g., xmlns:xi
            if (contains(get_structural_attributes(), attr_name) or attr_name.find(':') != string::npos)
                continue;
            if (attr_name == mx::InterfaceElement::NODE_DEF_ATTRIBUTE and element->isA<mx::NodeGraph>())
                continue;

            const string name = child_name.empty() ? attr_name : child_name + "." + attr_name;
            result.push_back("@" + name + " \"" + element->getAttribute(attr_name) + "\"");
        }
        return result;
    }

    optional<ExpressionCode> format_value(const mx::ValuePtr& value)
    {
        if (value == nullptr)
            return std::nullopt;

        // zero vectors and colors are written as their default value, e.g., `vec3{}`, and those whose components are all
        // the same as that component, e.g., `color3{1.0}`
        if (not get_swizzle_channels(value->getTypeString()).empty())
        {
            const vector<float> values = get_float_components(value);
            const bool is_uniform = not values.empty() and std::all_of(values.begin(), values.end(), [&](const float f) { return f == values.front(); });
            if (is_uniform)
            {
                const string type = get_type_alias(value->getTypeString());
                if (values.front() == 0.0f)
                    return decompile::format_constructor(type, {});
                return decompile::format_constructor(type, {decompile::format_literal(Primitive{values.front()}.to_string())});
            }
        }

        // matrices have no literal syntax, but the identity matrix is their default value
        const string& type = value->getTypeString();
        if ((type == "matrix33" and value->asA<mx::Matrix33>() == mx::Matrix33::IDENTITY) or (type == "matrix44" and value->asA<mx::Matrix44>() == mx::Matrix44::IDENTITY))
            return ExpressionCode{"default(" + get_type_alias(type) + ")"};

        const Primitive primitive{value};
        if (primitive.is_null())
            return std::nullopt;
        return decompile::format_literal(primitive.to_string());
    }

    optional<ExpressionCode> format_value(const mx::ValueElementPtr& element)
    {
        // the defaults of types without literal syntax, e.g., shaders, and of filenames, which would otherwise be strings
        const string& type = element->getType();
        if ((not has_literal_syntax(type) or type == "filename") and element->getValueString().empty())
            return ExpressionCode{"default(" + get_type_alias(type) + ")"};
        return format_value(element->getValue());
    }

    bool is_index(const string& field_name)
    {
        return not field_name.empty() and std::all_of(field_name.begin(), field_name.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    }

    bool is_separate(const mx::NodePtr& node)
    {
        const string& category = node->getCategory();
        return category == "separate2" or category == "separate3" or category == "separate4";
    }

    bool is_combine(const mx::NodePtr& node)
    {
        const string& category = node->getCategory();
        return category == "combine2" or category == "combine3" or category == "combine4";
    }

    size_t get_channel_count(const mx::NodePtr& node)
    {
        if (is_separate(node) or is_combine(node))
            return static_cast<size_t>(node->getCategory().back() - '0');
        return 0;
    }

    optional<char> get_separate_channel(const string& output_name)
    {
        if (output_name.size() == 4 and string_utils::starts_with(output_name, "out") and string{"xyzwrgba"}.find(output_name[3]) != string::npos)
            return output_name[3];
        return std::nullopt;
    }

    bool is_connected(const mx::InputPtr& input)
    {
        return input and (not input->getNodeName().empty() or not input->getNodeGraphString().empty() or not input->getInterfaceName().empty());
    }

    bool has_bool_value(const mx::InputPtr& input, const bool expected)
    {
        if (input == nullptr or is_connected(input) or not input->hasValue())
            return false;
        const mx::ValuePtr value = input->getValue();
        return value and value->isA<bool>() and value->asA<bool>() == expected;
    }

    optional<int> get_int_value(const mx::InputPtr& input)
    {
        if (input == nullptr or is_connected(input) or not input->hasValue())
            return std::nullopt;
        const mx::ValuePtr value = input->getValue();
        if (value and value->isA<int>())
            return value->asA<int>();
        return std::nullopt;
    }

    vector<float> get_float_components(const mx::ValuePtr& value)
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

    bool is_zero_value(const mx::ValuePtr& value)
    {
        if (value == nullptr)
            return false;
        if (value->isA<bool>())
            return not value->asA<bool>();
        if (value->isA<string>())
            return value->asA<string>().empty();

        // numeric values, e.g., "0" or "0, 0, 0"
        const vector<float> values = get_float_components(value);
        return not values.empty() and std::all_of(values.begin(), values.end(), [](const float f) { return f == 0.0f; });
    }

    bool is_zero_value(const mx::ValueElementPtr& element)
    {
        return is_zero_value(element->getValue());
    }

    optional<string> get_assigned_variable_name(const string& node_name)
    {
        if (not has_prefix(node_name, TEMPORARY_VARIABLE_PREFIX))
            return std::nullopt;

        // e.g., x__2 of var__x__2, variables can also contain double underscores, e.g., var__ray__origin__2
        const string rest = remove_prefix(node_name);
        const size_t split = rest.rfind("__");
        if (split == string::npos or split == 0)
            return std::nullopt;

        const string number = rest.substr(split + 2);
        if (number.empty() or not std::all_of(number.begin(), number.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); }))
            return std::nullopt;
        return rest.substr(0, split);
    }

    bool is_generated_node_name(const string& name)
    {
        return has_prefix(name, TEMPORARY_VARIABLE_PREFIX);
    }

    bool is_return_output(const mx::OutputPtr& output)
    {
        return not has_prefix(output, OUT_PARAMETER_PREFIX) and not has_prefix(output, NONLOCAL_OUT_PREFIX);
    }
}
