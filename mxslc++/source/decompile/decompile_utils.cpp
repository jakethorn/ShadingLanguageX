//
// Created by jaket on 28/09/2026.
//

#include "decompile/decompile_utils.h"

#include <functional>

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
        // the defaults of types without literal syntax, e.g., shaders
        const string& type = element->getType();
        if (not has_literal_syntax(type) and element->getValueString().empty())
            return ExpressionCode{"default(" + get_type_alias(type) + ")"};
        return format_value(element->getValue());
    }

    bool is_index(const string& field_name)
    {
        return not field_name.empty() and std::all_of(field_name.begin(), field_name.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    }

    string get_return_type(const vector<mx::OutputPtr>& outputs)
    {
        if (outputs.empty())
            return "void";
        if (outputs.size() == 1)
            return get_type_alias(outputs.front());

        string fields;
        for (const mx::OutputPtr& output : outputs)
        {
            const string field_name = without_prefix(output, RETURN_VALUE_PREFIX);
            fields += (fields.empty() ? "" : ", ") + get_type_alias(output);
            if (not is_index(field_name))
                fields += " " + make_identifier(field_name);
        }
        return "{" + fields + "}";
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

    namespace
    {
        // the number of a value that is assigned to a variable, e.g., 2 of var__x__2
        size_t get_assignment_number(const string& node_name)
        {
            return std::stoul(node_name.substr(node_name.rfind("__") + 2));
        }

        // the elements of the graph that the element is connected to or calls, e.g., the node def of the function that a
        // node calls, or the nodes that a node graph uses as default values and that a node def reads as nonlocal variables
        vector<mx::ElementPtr> get_dependencies(const mx::GraphElementPtr& graph, const mx::ElementPtr& element)
        {
            const mx::DocumentPtr document = graph->asA<mx::Document>();

            vector<mx::ElementPtr> dependencies;
            const auto add_connections = [&](const mx::PortElementPtr& port) {
                dependencies.push_back(graph->getNode(port->getNodeName()));
                if (document)
                    dependencies.push_back(document->getNodeGraph(port->getNodeGraphString()));
            };
            const auto add_calls = [&](const mx::NodeGraphPtr& body) {
                for (const mx::NodePtr& node : body->getNodes())
                    dependencies.push_back(node->getNodeDef());
            };
            const auto add_nonlocal_variable = [&](const mx::PortElementPtr& port) {
                if (has_prefix(port, NONLOCAL_IN_PREFIX) or has_prefix(port, NONLOCAL_OUT_PREFIX))
                    dependencies.push_back(graph->getNode(without_prefix(port)));
            };

            if (const mx::NodePtr node = element->asA<mx::Node>())
            {
                for (const mx::InputPtr& input : node->getInputs())
                    add_connections(input);
                if (document)
                    dependencies.push_back(node->getNodeDef());
            }
            else if (const mx::NodeDefPtr node_def = element->asA<mx::NodeDef>())
            {
                if (const mx::NodeGraphPtr body = mtlx_utils::get_node_graph(node_def))
                    add_calls(body);
                for (const mx::InputPtr& input : node_def->getActiveInputs())
                    add_nonlocal_variable(input);
                for (const mx::OutputPtr& output : node_def->getActiveOutputs())
                    add_nonlocal_variable(output);
            }
            else if (const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>())
            {
                // the node graphs of node defs are their bodies
                if (const mx::NodeDefPtr implemented = node_graph->getNodeDef())
                {
                    dependencies.push_back(implemented);
                }
                else
                {
                    add_calls(node_graph);
                    for (const mx::InputPtr& input : node_graph->getInputs())
                        add_connections(input);
                }
            }
            else if (const mx::OutputPtr output = element->asA<mx::Output>())
            {
                add_connections(output);
            }

            // only the elements of the graph are sorted, e.g., not the node defs of the MaterialX libraries
            const auto is_outside = [&](const mx::ElementPtr& dependency) { return dependency == nullptr or dependency == element or dependency->getParent() != graph; };
            dependencies.erase(std::remove_if(dependencies.begin(), dependencies.end(), is_outside), dependencies.end());
            return dependencies;
        }

        // true if the node is a temporary value, var__<n>, which is written as part of the expression that uses it
        bool is_temporary_node(const mx::NodePtr& node)
        {
            return is_generated_node_name(node->getName()) and not get_assigned_variable_name(node->getName());
        }

        // true if the call assigns a new value to the variable of the argument, e.g., the argument of a ref parameter, or a
        // nonlocal variable that the function assigns to
        bool is_overwritten_argument(const mx::NodeDefPtr& node_def, const mx::InputPtr& input)
        {
            const string& name = input->getName();
            if (has_prefix(name, NONLOCAL_IN_PREFIX))
                return node_def->getActiveOutput(with_prefix(NONLOCAL_OUT_PREFIX, remove_prefix(name))) != nullptr;
            return node_def->getActiveOutput(with_prefix(OUT_PARAMETER_PREFIX, name)) != nullptr;
        }

        // the node and the nodes that use it, directly or through other nodes
        unordered_set<mx::NodePtr> find_dependents(const mx::NodePtr& node, const unordered_map<mx::NodePtr, vector<mx::NodePtr>>& uses)
        {
            unordered_set<mx::NodePtr> dependents{node};
            vector<mx::NodePtr> stack{node};
            while (not stack.empty())
            {
                const mx::NodePtr current = stack.back();
                stack.pop_back();
                if (not contains(uses, current))
                    continue;
                for (const mx::NodePtr& use : uses.at(current))
                {
                    if (dependents.insert(use).second)
                        stack.push_back(use);
                }
            }
            return dependents;
        }

        // The values that are assigned to variables, which overwrite the previous values of the variables, come after the
        // previous values and the code that uses them, in the code that the graph is compiled from. The previous value of
        // a value named var__x__2 is var__x__1, and of a call that assigns to an argument it is the argument. The code that
        // uses a previous value includes the expressions that use it through temporary values, e.g., `y` of
        // `float y = x * 2.0 + 1.0;`, because temporary values are written as part of the expressions that use them.
        unordered_map<mx::ElementPtr, vector<mx::ElementPtr>> find_assignment_dependencies(const mx::GraphElementPtr& graph)
        {
            unordered_map<mx::NodePtr, vector<mx::NodePtr>> uses;
            vector<std::pair<mx::NodePtr, mx::NodePtr>> overwritten_values;
            unordered_map<string, vector<mx::NodePtr>> variable_values;
            for (const mx::NodePtr& node : graph->getNodes())
            {
                const mx::NodeDefPtr node_def = node->getNodeDef();
                for (const mx::InputPtr& input : node->getInputs())
                {
                    const mx::NodePtr used = graph->getNode(input->getNodeName());
                    if (used == nullptr)
                        continue;
                    uses[used].push_back(node);
                    if (node_def and is_overwritten_argument(node_def, input))
                        overwritten_values.emplace_back(used, node);
                }

                if (const optional<string> variable = get_assigned_variable_name(node->getName()))
                    variable_values[*variable].push_back(node);
            }

            for (auto& [variable, nodes] : variable_values)
            {
                std::sort(nodes.begin(), nodes.end(), [](const mx::NodePtr& a, const mx::NodePtr& b) {
                    return get_assignment_number(a->getName()) < get_assignment_number(b->getName());
                });

                mx::NodePtr previous = graph->getNode(variable);
                for (const mx::NodePtr& node : nodes)
                {
                    if (previous)
                        overwritten_values.emplace_back(previous, node);
                    previous = node;
                }
            }

            unordered_map<mx::ElementPtr, vector<mx::ElementPtr>> dependencies;
            for (const auto& [previous, node] : overwritten_values)
            {
                vector<mx::ElementPtr>& node_dependencies = dependencies[node];
                node_dependencies.push_back(previous);

                // the code that uses the new value cannot come before it
                const unordered_set<mx::NodePtr> dependents = find_dependents(node, uses);
                unordered_set<mx::NodePtr> visited;
                vector<mx::NodePtr> stack = uses[previous];
                while (not stack.empty())
                {
                    const mx::NodePtr use = stack.back();
                    stack.pop_back();
                    if (contains(dependents, use) or not visited.insert(use).second)
                        continue;

                    node_dependencies.push_back(use);
                    if (is_temporary_node(use) and contains(uses, use))
                        stack.insert(stack.end(), uses.at(use).begin(), uses.at(use).end());
                }
            }
            return dependencies;
        }

        // the elements after the elements that they depend on, in their order where possible, or nothing if their
        // dependencies are cyclic
        optional<vector<mx::ElementPtr>> sort_elements(const mx::GraphElementPtr& graph, const vector<mx::ElementPtr>& elements, const unordered_map<mx::ElementPtr, vector<mx::ElementPtr>>& assignment_dependencies)
        {
            vector<mx::ElementPtr> result;
            unordered_set<mx::ElementPtr> visited;
            unordered_set<mx::ElementPtr> visiting;
            bool is_cyclic = false;

            const std::function<void(const mx::ElementPtr&)> visit = [&](const mx::ElementPtr& element) {
                if (contains(visiting, element))
                    is_cyclic = true;
                if (contains(visited, element))
                    return;
                visited.insert(element);
                visiting.insert(element);

                if (contains(assignment_dependencies, element))
                {
                    for (const mx::ElementPtr& dependency : assignment_dependencies.at(element))
                        visit(dependency);
                }
                for (const mx::ElementPtr& dependency : get_dependencies(graph, element))
                    visit(dependency);

                visiting.erase(element);
                result.push_back(element);
            };

            for (const mx::ElementPtr& element : elements)
                visit(element);

            if (is_cyclic)
                return std::nullopt;
            return result;
        }

        // the order of the values of variables is only known from their names, which the graph does not have to follow,
        // so it is ignored if it conflicts with the connections of the graph
        vector<mx::ElementPtr> sort_elements(const mx::GraphElementPtr& graph, const vector<mx::ElementPtr>& elements)
        {
            if (optional<vector<mx::ElementPtr>> result = sort_elements(graph, elements, find_assignment_dependencies(graph)))
                return *result;
            if (optional<vector<mx::ElementPtr>> result = sort_elements(graph, elements, {}))
                return *result;
            return elements;
        }

        void set_child_order(const mx::ElementPtr& parent, const vector<mx::ElementPtr>& children)
        {
            for (size_t i = 0; i < children.size(); ++i)
                parent->setChildIndex(children[i]->getName(), static_cast<int>(i));
        }
    }

    void sort_by_dependencies(const mx::DocumentPtr& document)
    {
        set_child_order(document, sort_elements(document, document->getChildren()));

        // the nodes of node graphs are sorted in the places of the nodes, e.g., between its inputs and outputs
        for (const mx::NodeGraphPtr& node_graph : document->getNodeGraphs())
        {
            const vector<mx::NodePtr> nodes = node_graph->getNodes();
            const vector<mx::ElementPtr> sorted_nodes = sort_elements(node_graph, {nodes.begin(), nodes.end()});

            vector<mx::ElementPtr> children;
            size_t next_node = 0;
            for (const mx::ElementPtr& child : node_graph->getChildren())
                children.push_back(child->isA<mx::Node>() ? sorted_nodes.at(next_node++) : child);
            set_child_order(node_graph, children);
        }
    }
}
