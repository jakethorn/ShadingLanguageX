//
// Created by jaket on 28/09/2026.
//

#include "decompile/GraphDecompiler.h"

#include <functional>

#include "decompile/Decompiler.h"
#include "decompile/decompile_utils.h"
#include "serialize/name_prefix_utils.h"
#include "utils/container_utils.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace decompile_utils;
    using container_utils::contains;

    namespace
    {
        bool is_integer_arithmetic(const mx::NodePtr& node)
        {
            // integer multiply, divide, modulo and power are not compiled to a single node, see stdlib.mxsl
            const string& category = node->getCategory();
            const bool is_int_op = category == "multiply" or category == "divide" or category == "modulo" or category == "power";
            return is_int_op and node->getType() == "integer";
        }

        const unordered_map<string, string>& get_binary_operators()
        {
            static const unordered_map<string, string> operators {
                {"add", "+"},
                {"subtract", "-"},
                {"multiply", "*"},
                {"divide", "/"},
                {"modulo", "%"},
                {"power", "^"},
                {"and", "&"},
                {"or", "|"},
                {"xor", "^"},
            };
            return operators;
        }

        // the valid argument types of the multi-argument constructors in stdlib.mxsl, excluding all floats
        bool is_constructor_signature(const string& type_name, const vector<string>& arg_types)
        {
            static const unordered_map<string, vector<vector<string>>> signatures {
                {"vector3", {{"float", "vector2"}, {"vector2", "float"}}},
                {"color3", {{"float", "vector2"}, {"vector2", "float"}}},
                {"vector4", {
                    {"float", "float", "vector2"}, {"float", "vector2", "float"}, {"vector2", "float", "float"},
                    {"float", "vector3"}, {"float", "color3"}, {"vector3", "float"}, {"color3", "float"}
                }},
                {"color4", {
                    {"float", "float", "vector2"}, {"float", "vector2", "float"}, {"vector2", "float", "float"},
                    {"float", "vector3"}, {"float", "color3"}, {"vector3", "float"}, {"color3", "float"}
                }},
            };

            if (not contains(signatures, type_name))
                return false;
            return contains(signatures.at(type_name), arg_types);
        }

        // a field of a struct, which is indexed if the struct has no field names, e.g., `f[0]`
        ExpressionCode format_field_access(const ExpressionCode& value, const string& field_name)
        {
            if (is_index(field_name))
                return format_indexing(value, format_literal(field_name));
            return format_member_access(value, make_identifier(field_name));
        }

        // sets a member of the decompiler while an expression is created, e.g., whether its type is known
        template<typename T>
        class ScopedValue
        {
        public:
            ScopedValue(T& member, T value) : member_{member}, saved_{member} { member_ = std::move(value); }
            ~ScopedValue() { member_ = saved_; }
            ScopedValue(const ScopedValue&) = delete;
            ScopedValue& operator=(const ScopedValue&) = delete;

        private:
            T& member_;
            T saved_;
        };

        using TypedContext = ScopedValue<bool>;

        // e.g., the 1.0 of `x + 1.0`, which is written `x++;` if it is assigned to x
        bool is_one(const mx::InputPtr& input)
        {
            if (input == nullptr or is_connected(input) or not input->hasValue())
                return false;
            const mx::ValuePtr value = input->getValue();
            return (value->isA<float>() and value->asA<float>() == 1.0f) or (value->isA<int>() and value->asA<int>() == 1);
        }

        SourceCode format_variable_definition(const string& mods, const string& type, const string& name, const optional<ExpressionCode>& value)
        {
            string declaration = mods.empty() ? "" : mods + " ";
            declaration += type + " " + name;
            if (not value)
                return declaration + ";";
            return SourceCode::concat({declaration + " = ", value->code, ";"});
        }

        SourceCode format_expression_statement(const ExpressionCode& expr)
        {
            // statements beginning with `if` are if statements, so if-expressions are wrapped in parentheses
            if (expr.is_if_expression())
                return SourceCode::concat({"(", expr.code, ");"});
            return SourceCode::concat({expr.code, ";"});
        }
    }

    GraphDecompiler::GraphDecompiler(Decompiler& decompiler, mx::GraphElementPtr graph, unordered_set<string> outside_variables)
        : decompiler_{decompiler}, graph_{std::move(graph)}, nodes_{graph_->getNodes()}, outside_variables_{std::move(outside_variables)}
    {
        for (const mx::NodePtr& node : nodes_)
        {
            for (const mx::InputPtr& input : node->getInputs())
                add_use(input, node);
        }

        for (const mx::OutputPtr& output : graph_->getOutputs())
            add_use(output, output);

        // node graphs can connect their inputs to nodes in the document
        if (graph_->isA<mx::Document>())
        {
            for (const mx::NodeGraphPtr& node_graph : graph_->asA<mx::Document>()->getNodeGraphs())
            {
                for (const mx::InputPtr& input : node_graph->getInputs())
                    add_use(input, input);
            }
        }

        find_assignments();
        do
        {
            find_absorbed_nodes();
            find_statements();
            create_identifiers();
        }
        while (remove_invalid_assignment());
    }

    bool GraphDecompiler::is_statement(const mx::NodePtr& node) const
    {
        return contains(statements_, node);
    }

    size_t GraphDecompiler::get_use_count(const mx::NodePtr& node) const
    {
        if (contains(uses_, node))
            return uses_.at(node).size();
        return 0;
    }

    optional<string> GraphDecompiler::get_outside_variable(const mx::NodePtr& node) const
    {
        const VariableAssignments* assignments = get_assignments(node);
        if (assignments and assignments->is_declared_outside)
            return assignments->variable;
        return std::nullopt;
    }

    bool GraphDecompiler::is_assigned(const string& variable) const
    {
        return std::any_of(assignments_.begin(), assignments_.end(), [&](const VariableAssignments& assignments) {
            return assignments.variable == variable and not assignments.nodes.empty();
        });
    }

    bool GraphDecompiler::has_assigned_value(const mx::PortElementPtr& port, const string& variable) const
    {
        const mx::NodePtr node = graph_->getNode(port->getNodeName());
        const VariableAssignments* assignments = node ? get_assignments(node) : nullptr;
        return assignments and assignments->identifier == variable;
    }

    void GraphDecompiler::find_assignments()
    {
        unordered_map<mx::NodePtr, size_t> positions;
        for (size_t i = 0; i < nodes_.size(); ++i)
            positions[nodes_[i]] = i;

        // the values that the compiler named after the variable that they are assigned to, e.g., var__x__1
        unordered_map<string, size_t> indices;
        for (const mx::NodePtr& node : nodes_)
        {
            const optional<string> variable = get_assigned_variable_name(node->getName());
            if (not variable or node->getType() == mx::MULTI_OUTPUT_TYPE_STRING)
                continue;

            if (not contains(indices, *variable))
            {
                indices[*variable] = assignments_.size();
                assignments_.push_back(VariableAssignments{*variable, {}, contains(outside_variables_, make_identifier(*variable))});
            }
            assignments_[indices.at(*variable)].nodes.push_back(node);
        }

        for (VariableAssignments& assignments : assignments_)
        {
            // the first value of a variable is the node of its definition, which is named after it, e.g., x, unless it
            // has no node, e.g., `mutable float x = 0.0;`
            if (const mx::NodePtr definition = graph_->getNode(assignments.variable); definition and not assignments.is_declared_outside)
            {
                // definitions that cannot be written as the declaration of a mutable variable keep their later values
                // as separate values, e.g., `geomprop float x;`
                if (definition->getType() == mx::MULTI_OUTPUT_TYPE_STRING or is_geomprop_definition(definition))
                {
                    assignments.nodes.clear();
                    continue;
                }
                assignments.nodes.push_back(definition);
            }

            std::sort(assignments.nodes.begin(), assignments.nodes.end(), [&](const mx::NodePtr& a, const mx::NodePtr& b) {
                return positions.at(a) < positions.at(b);
            });

            // a variable has a single type, other values, e.g., of a local variable of an inline function that is
            // called with different types, are separate values
            const string type = assignments.nodes.front()->getType();
            const auto is_other_type = [&](const mx::NodePtr& node) { return node->getType() != type; };
            assignments.nodes.erase(std::remove_if(assignments.nodes.begin(), assignments.nodes.end(), is_other_type), assignments.nodes.end());
        }

        for (size_t i = 0; i < assignments_.size(); ++i)
        {
            for (const mx::NodePtr& node : assignments_[i].nodes)
                assignment_indices_[node] = i;
        }
    }

    const GraphDecompiler::VariableAssignments* GraphDecompiler::get_assignments(const mx::NodePtr& node) const
    {
        if (contains(assignment_indices_, node))
            return &assignments_.at(assignment_indices_.at(node));
        return nullptr;
    }

    mx::NodePtr GraphDecompiler::get_previous_value(const mx::NodePtr& node) const
    {
        const VariableAssignments* assignments = get_assignments(node);
        if (assignments == nullptr)
            return nullptr;

        const auto it = std::find(assignments->nodes.begin(), assignments->nodes.end(), node);
        return it == assignments->nodes.begin() ? nullptr : *(it - 1);
    }

    bool GraphDecompiler::is_previous_value(const mx::NodePtr& node, const mx::InputPtr& input) const
    {
        if (input == nullptr)
            return false;
        if (const mx::NodePtr previous = get_previous_value(node))
            return get_connected_node(input) == previous;

        // the first value assigned to a parameter or nonlocal variable, which the graph gets from its interface
        const VariableAssignments* assignments = get_assignments(node);
        const string& interface_name = input->getInterfaceName();
        if (assignments == nullptr or not assignments->is_declared_outside or interface_name.empty())
            return false;
        const bool is_nonlocal = has_prefix(interface_name, NONLOCAL_IN_PREFIX);
        return (is_nonlocal ? remove_prefix(interface_name) : interface_name) == assignments->variable;
    }

    void GraphDecompiler::remove_assignment(const mx::NodePtr& node)
    {
        VariableAssignments& assignments = assignments_.at(assignment_indices_.at(node));
        assignments.nodes.erase(std::find(assignments.nodes.begin(), assignments.nodes.end(), node));
        assignment_indices_.erase(node);
    }

    bool GraphDecompiler::remove_invalid_assignment()
    {
        const vector<mx::NodePtr> order = get_ordered_statements();
        unordered_map<mx::NodePtr, size_t> positions;
        for (size_t i = 0; i < order.size(); ++i)
            positions[order[i]] = i;

        // the value of the variable before the statement at the position, or null if nothing is assigned to it yet
        const auto current_value = [&](const VariableAssignments& assignments, const size_t position) {
            mx::NodePtr result;
            for (const mx::NodePtr& node : assignments.nodes)
            {
                if (positions.at(node) < position and (result == nullptr or positions.at(node) > positions.at(result)))
                    result = node;
            }
            return result;
        };

        // the values that are used at the position must be the values that their variables have at that position
        const auto remove_overwritten_value = [&](const vector<mx::NodePtr>& dependencies, const unordered_set<string>& interface_names, const size_t position) {
            for (const mx::NodePtr& dependency : dependencies)
            {
                const VariableAssignments* assignments = get_assignments(dependency);
                if (assignments == nullptr)
                    continue;

                const mx::NodePtr current = current_value(*assignments, position);
                if (current == dependency)
                    continue;

                // the definition of a variable keeps its name, so the value that overwrites it is separated instead
                remove_assignment(dependency->getName() == assignments->variable ? current : dependency);
                return true;
            }

            // parameters and nonlocal variables cannot be used as the value of the interface after they are assigned
            for (const VariableAssignments& assignments : assignments_)
            {
                if (not assignments.is_declared_outside or not contains(interface_names, assignments.variable))
                    continue;
                if (const mx::NodePtr current = current_value(assignments, position))
                {
                    remove_assignment(current);
                    return true;
                }
            }

            return false;
        };

        for (size_t i = 0; i < order.size(); ++i)
        {
            unordered_set<string> interface_names;
            unordered_set<mx::NodePtr> visited;
            for (const mx::InputPtr& input : order[i]->getInputs())
                collect_interface_dependencies(input, interface_names, visited);

            if (remove_overwritten_value(get_statement_dependencies(order[i]), interface_names, i))
                return true;
        }

        // the outputs of the graph are the values after all of its statements, e.g., the return value of a function
        for (const mx::OutputPtr& output : graph_->getOutputs())
        {
            vector<mx::NodePtr> statements;
            vector<mx::ElementPtr> functions;
            unordered_set<mx::NodePtr> visited;
            collect_dependencies(output, statements, functions, visited);

            if (remove_overwritten_value(statements, get_interface_dependencies(output), order.size()))
                return true;
        }

        return false;
    }

    unordered_set<string> GraphDecompiler::get_interface_dependencies(const mx::PortElementPtr& port) const
    {
        unordered_set<string> names;
        unordered_set<mx::NodePtr> visited;
        collect_interface_dependencies(port, names, visited);
        return names;
    }

    void GraphDecompiler::collect_interface_dependencies(const mx::PortElementPtr& port, unordered_set<string>& names, unordered_set<mx::NodePtr>& visited) const
    {
        // nonlocal variables are read through inputs named nonlocal_in__<name>
        if (const string& interface_name = port->getAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE); not interface_name.empty())
        {
            const bool is_nonlocal = has_prefix(interface_name, NONLOCAL_IN_PREFIX);
            names.insert(is_nonlocal ? remove_prefix(interface_name) : interface_name);
        }

        // statements are checked separately
        const mx::NodePtr node = port->getNodeName().empty() ? nullptr : graph_->getNode(port->getNodeName());
        if (node == nullptr or is_statement(node) or contains(visited, node))
            return;

        visited.insert(node);
        for (const mx::InputPtr& input : node->getInputs())
            collect_interface_dependencies(input, names, visited);
    }

    vector<mx::NodePtr> GraphDecompiler::get_ordered_statements() const
    {
        vector<mx::NodePtr> result;
        unordered_set<mx::NodePtr> visited;

        std::function<void(const mx::NodePtr&)> visit = [&](const mx::NodePtr& node) {
            if (contains(visited, node))
                return;
            visited.insert(node);
            for (const mx::NodePtr& dependency : get_statement_dependencies(node))
                visit(dependency);
            result.push_back(node);
        };

        for (const mx::NodePtr& node : nodes_)
        {
            if (is_statement(node))
                visit(node);
        }

        return result;
    }

    vector<mx::NodePtr> GraphDecompiler::get_statement_dependencies(const mx::NodePtr& node) const
    {
        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited{node};
        for (const mx::InputPtr& input : node->getInputs())
            collect_dependencies(input, statements, functions, visited);
        return statements;
    }

    vector<mx::ElementPtr> GraphDecompiler::get_function_dependencies(const mx::NodePtr& node) const
    {
        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited{node};
        collect_dependencies(node, statements, functions, visited);
        return functions;
    }

    vector<mx::ElementPtr> GraphDecompiler::get_function_dependencies(const mx::PortElementPtr& port) const
    {
        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited;
        collect_dependencies(port, statements, functions, visited);
        return functions;
    }

    void GraphDecompiler::collect_dependencies(const mx::NodePtr& node, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def and decompiler_.is_document_node_def(node_def) and not contains(functions, node_def))
            functions.push_back(node_def);

        for (const mx::InputPtr& input : node->getInputs())
            collect_dependencies(input, statements, functions, visited);
    }

    void GraphDecompiler::collect_dependencies(const mx::PortElementPtr& port, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const
    {
        if (const string& node_graph_name = port->getNodeGraphString(); not node_graph_name.empty())
        {
            const mx::NodeGraphPtr node_graph = decompiler_.document()->getNodeGraph(node_graph_name);
            if (node_graph and not contains(functions, node_graph))
                functions.push_back(node_graph);
        }

        const mx::NodePtr node = port->isA<mx::Input>() ? get_connected_node(port->asA<mx::Input>()) : graph_->getNode(port->getNodeName());
        if (node == nullptr or contains(visited, node))
            return;

        if (is_statement(node))
        {
            if (not contains(statements, node))
                statements.push_back(node);
            // statements are declared separately, but their functions are still dependencies of this expression
            const mx::NodeDefPtr node_def = node->getNodeDef();
            if (node_def and decompiler_.is_document_node_def(node_def) and not contains(functions, node_def))
                functions.push_back(node_def);
            return;
        }

        visited.insert(node);
        collect_dependencies(node, statements, functions, visited);
    }

    void GraphDecompiler::add_use(const mx::PortElementPtr& port, const mx::ElementPtr& consumer)
    {
        if (port->getNodeName().empty())
            return;

        const mx::NodePtr node = graph_->getNode(port->getNodeName());
        if (node == nullptr)
            return;

        uses_[node].push_back(NodeUse{consumer, port->getOutputString()});
    }

    bool GraphDecompiler::is_absorbable_by(const mx::NodePtr& helper, const mx::NodePtr& consumer, const size_t expected_uses) const
    {
        // the values of variables are always written as statements
        if (helper == nullptr or helper == consumer or not contains(uses_, helper) or get_assignments(helper))
            return false;

        const vector<NodeUse>& uses = uses_.at(helper);
        if (uses.size() != expected_uses)
            return false;

        for (const NodeUse& use : uses)
        {
            if (use.consumer != consumer)
                return false;
        }

        return get_user_attributes(helper).empty();
    }

    mx::NodePtr GraphDecompiler::get_connected_node(const mx::InputPtr& input) const
    {
        if (input == nullptr or input->getNodeName().empty())
            return nullptr;
        return graph_->getNode(input->getNodeName());
    }

    // `1.0 - v` is compiled to `subtract(convert(1.0), v)`
    mx::NodePtr GraphDecompiler::get_float_operand_convert(const mx::NodePtr& node) const
    {
        const string& category = node->getCategory();
        if (category != "subtract" and category != "divide" and category != "modulo" and category != "power")
            return nullptr;

        const mx::NodePtr convert = get_connected_node(node->getInput("in1"));
        if (convert == nullptr or convert->getCategory() != "convert" or convert->getType() != node->getType())
            return nullptr;

        const mx::InputPtr convert_input = convert->getInput("in");
        if (convert_input == nullptr or convert_input->getType() != "float")
            return nullptr;

        return is_absorbable_by(convert, node, 1) ? convert : nullptr;
    }

    // `v.zx` is compiled to `combine2(separate3(v).outz, separate3(v).outx)`, v can also be a value, e.g., `vec3{}.zx`
    mx::NodePtr GraphDecompiler::get_swizzle_separate(const mx::NodePtr& node) const
    {
        if (not is_combine(node) or (node->getType() == "vector4" and node->getCategory() == "combine2"))
            return nullptr;

        const bool is_color = is_color_type(node->getType());
        const size_t count = get_channel_count(node);

        mx::NodePtr separate;
        for (size_t i = 1; i <= count; ++i)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i));
            const mx::NodePtr input_node = get_connected_node(input);
            if (input_node == nullptr or not is_separate(input_node))
                return nullptr;
            if (separate and input_node != separate)
                return nullptr;
            separate = input_node;
            if (separate->getInput("in") == nullptr)
                return nullptr;

            const optional<char> channel = get_separate_channel(input->getOutputString());
            if (not channel)
                return nullptr;
            const bool is_color_channel = string{"rgba"}.find(*channel) != string::npos;
            if (is_color_channel != is_color)
                return nullptr;
        }

        return is_absorbable_by(separate, node, count) ? separate : nullptr;
    }

    // `vec3{uv, 1.0}` is compiled to `combine3(separate2(uv).outx, separate2(uv).outy, 1.0)`
    vector<GraphDecompiler::SeparatedArgument> GraphDecompiler::get_separated_arguments(const mx::NodePtr& node) const
    {
        if (node->getCategory() != "combine3" and node->getCategory() != "combine4")
            return {};

        const size_t count = get_channel_count(node);

        vector<SeparatedArgument> arguments;
        vector<string> arg_types;
        size_t i = 0;
        while (i < count)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i + 1));
            const mx::NodePtr separate = get_connected_node(input);

            bool is_argument = separate and is_separate(separate) and i + get_channel_count(separate) <= count;
            const size_t argument_channel_count = is_argument ? get_channel_count(separate) : 0;
            for (size_t j = 0; is_argument and j < argument_channel_count; ++j)
            {
                const mx::InputPtr argument_input = node->getInput("in" + std::to_string(i + j + 1));
                const optional<char> channel = get_separate_channel(argument_input->getOutputString());
                const bool is_in_order = channel and (*channel == "xyzw"[j] or *channel == "rgba"[j]);
                is_argument = get_connected_node(argument_input) == separate and is_in_order;
            }
            is_argument = is_argument and is_absorbable_by(separate, node, argument_channel_count);

            if (is_argument)
            {
                const mx::InputPtr separate_input = separate->getInput("in");
                if (separate_input == nullptr)
                    return {};
                arguments.push_back(SeparatedArgument{i, argument_channel_count, separate});
                arg_types.push_back(separate_input->getType());
                i += argument_channel_count;
            }
            else
            {
                arg_types.emplace_back("float");
                ++i;
            }
        }

        if (arguments.empty() or not is_constructor_signature(node->getType(), arg_types))
            return {};
        return arguments;
    }

    optional<GraphDecompiler::SwizzleAssignment> GraphDecompiler::find_swizzle_assignment(const mx::NodePtr& node) const
    {
        const mx::NodePtr previous = get_previous_value(node);
        const string channels = get_swizzle_channels(node->getType());
        if (previous == nullptr or not is_combine(node) or channels.size() != get_channel_count(node))
            return std::nullopt;

        // the channels that keep the previous value of the variable, which come from a single separate node
        mx::NodePtr previous_separate;
        size_t kept_count = 0;
        vector<size_t> assigned_channels;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i + 1));
            const mx::NodePtr separate = get_connected_node(input);
            const bool is_kept = separate and is_separate(separate) and get_connected_node(separate->getInput("in")) == previous and
                get_separate_channel(input->getOutputString()) == channels[i] and (previous_separate == nullptr or separate == previous_separate);

            if (is_kept)
            {
                previous_separate = separate;
                ++kept_count;
            }
            else
            {
                assigned_channels.push_back(i);
            }
        }

        if (assigned_channels.empty() or not is_absorbable_by(previous_separate, node, kept_count))
            return std::nullopt;

        if (assigned_channels.size() == 1)
        {
            const size_t channel = assigned_channels.front();
            return SwizzleAssignment{previous_separate, nullptr, "in" + std::to_string(channel + 1), string{channels[channel]}};
        }

        // `q.xz = v;` is compiled to `combine3(separate2(v).outx, separate3(q).outy, separate2(v).outy)`
        const mx::NodePtr value_separate = get_connected_node(node->getInput("in" + std::to_string(assigned_channels.front() + 1)));
        if (value_separate == nullptr or not is_separate(value_separate) or value_separate->getInput("in") == nullptr)
            return std::nullopt;
        if (get_channel_count(value_separate) != assigned_channels.size() or not is_absorbable_by(value_separate, node, assigned_channels.size()))
            return std::nullopt;

        // the channels of the swizzle are in the order of the channels of the value
        const string value_channels = get_swizzle_channels(value_separate->getInput("in")->getType());
        string swizzle(assigned_channels.size(), ' ');
        for (const size_t i : assigned_channels)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i + 1));
            if (get_connected_node(input) != value_separate)
                return std::nullopt;

            const optional<char> value_channel = get_separate_channel(input->getOutputString());
            const size_t index = value_channel ? value_channels.find(*value_channel) : string::npos;
            if (index >= swizzle.size() or swizzle[index] != ' ')
                return std::nullopt;
            swizzle[index] = channels[i];
        }

        return SwizzleAssignment{previous_separate, value_separate, "", swizzle};
    }

    optional<string> GraphDecompiler::get_compound_operator(const mx::NodePtr& node) const
    {
        const string& category = node->getCategory();
        if (not contains(get_binary_operators(), category) or is_integer_arithmetic(node) or get_float_operand_convert(node))
            return std::nullopt;
        if (not is_previous_value(node, node->getInput("in1")))
            return std::nullopt;
        return get_binary_operators().at(category);
    }

    // `a > b` is compiled to `ifgreater(a, b)` with a boolean output
    bool GraphDecompiler::is_comparison(const mx::NodePtr& node) const
    {
        const string& category = node->getCategory();
        if (category != "ifgreater" and category != "ifgreatereq" and category != "ifequal")
            return false;
        return node->getType() == "boolean" and node->getInput("in1") == nullptr and node->getInput("in2") == nullptr;
    }

    // `if (c) { a } else { b }` is compiled to `ifequal(c, true, a, b)`
    bool GraphDecompiler::is_if_expression(const mx::NodePtr& node) const
    {
        if (node->getCategory() != "ifequal")
            return false;

        const mx::InputPtr condition = node->getInput("value1");
        if (not is_connected(condition) or condition->getType() != "boolean" or not has_bool_value(node->getInput("value2"), true))
            return false;

        return node->getInput("in1") != nullptr or node->getInput("in2") != nullptr;
    }

    // `geomprop float x;` is compiled to `geompropvalue("x")` named x
    bool GraphDecompiler::is_geomprop_definition(const mx::NodePtr& node) const
    {
        if (node->getCategory() != "geompropvalue" or is_generated_node_name(node->getName()) or not is_valid_identifier(node->getName()))
            return false;

        const mx::InputPtr geomprop = node->getInput("geomprop");
        if (geomprop == nullptr or not geomprop->getNodeName().empty() or not geomprop->hasValue() or geomprop->getValueString() != node->getName())
            return false;

        for (const mx::InputPtr& input : node->getInputs())
        {
            if (input->getName() != "geomprop" and input->getName() != "default")
                return false;
        }

        return get_user_attributes(geomprop).empty();
    }

    // `return 1.0;` is compiled to an output connected to `constant(1.0)`
    bool GraphDecompiler::is_output_constant(const mx::NodePtr& node) const
    {
        if (not graph_->isA<mx::NodeGraph>() or node->getCategory() != "constant" or get_use_count(node) != 1)
            return false;
        if (not uses_.at(node).front().consumer->isA<mx::Output>())
            return false;

        const mx::InputPtr value = node->getInput("value");
        if (value == nullptr or is_connected(value) or not value->hasValue() or not has_literal_syntax(value->getType()))
            return false;

        return get_user_attributes(node).empty() and get_user_attributes(value).empty() and node->getInputs().size() == 1;
    }

    void GraphDecompiler::find_absorbed_nodes()
    {
        absorbed_.clear();
        swizzle_assignments_.clear();

        for (const mx::NodePtr& node : nodes_)
        {
            if (const optional<SwizzleAssignment> assignment = find_swizzle_assignment(node))
            {
                absorbed_.insert(assignment->previous_separate);
                if (assignment->value_separate)
                    absorbed_.insert(assignment->value_separate);
                swizzle_assignments_[node] = *assignment;
                continue;
            }

            if (const mx::NodePtr convert = get_float_operand_convert(node))
                absorbed_.insert(convert);

            if (const mx::NodePtr separate = get_swizzle_separate(node))
            {
                absorbed_.insert(separate);
            }
            else
            {
                for (const SeparatedArgument& argument : get_separated_arguments(node))
                    absorbed_.insert(argument.separate);
            }
        }
    }

    void GraphDecompiler::find_statements()
    {
        statements_.clear();

        for (const mx::NodePtr& node : nodes_)
        {
            // the values of variables, e.g., `x = x * 2.0;`
            if (get_assignments(node))
            {
                statements_.insert(node);
                continue;
            }

            if (contains(absorbed_, node) or is_output_constant(node))
                continue;

            const bool is_named = not is_generated_node_name(node->getName());
            const bool is_shared = get_use_count(node) != 1;
            const bool has_attributes = not get_user_attributes(node).empty();
            const bool has_multiple_outputs = node->getType() == mx::MULTI_OUTPUT_TYPE_STRING;
            const bool has_out_parameters = not get_out_parameter_outputs(node).empty();
            // e.g., void functions that assign to nonlocal variables
            const bool has_no_return_value = get_return_outputs(node).empty();

            if (is_named or is_shared or has_attributes or has_multiple_outputs or has_out_parameters or has_no_return_value)
                statements_.insert(node);
        }
    }

    void GraphDecompiler::create_identifiers()
    {
        used_identifiers_ = outside_variables_;
        identifiers_.clear();
        output_identifiers_.clear();

        // the variables that are declared outside of the graph use their own names
        for (VariableAssignments& assignments : assignments_)
            assignments.identifier = assignments.is_declared_outside ? make_identifier(assignments.variable) : "";

        // variables and named nodes take priority over the names generated for temporaries
        for (const mx::NodePtr& node : nodes_)
        {
            if (contains(assignment_indices_, node))
            {
                VariableAssignments& assignments = assignments_.at(assignment_indices_.at(node));
                if (assignments.identifier.empty())
                    assignments.identifier = create_unique_identifier(make_identifier(assignments.variable));
                identifiers_[node] = assignments.identifier;
            }
            else if (not is_generated_node_name(node->getName()))
            {
                identifiers_[node] = create_unique_identifier(make_identifier(node->getName()));
            }
        }

        for (const mx::NodePtr& node : nodes_)
        {
            if (contains(identifiers_, node))
                continue;

            // e.g., var_3 for var__3, and x_2 for var__x__2 if it cannot be written as an assignment to x
            const string& name = node->getName();
            if (const optional<string> variable = get_assigned_variable_name(name))
                identifiers_[node] = create_unique_identifier(make_identifier(*variable + "_" + name.substr(name.rfind("__") + 2)));
            else
                identifiers_[node] = create_unique_identifier("var_" + name.substr(5));
        }

        // out arguments are declared as part of the function call, e.g., `sincos(x, float s, float c);`
        for (const mx::NodePtr& node : nodes_)
        {
            for (const mx::OutputPtr& output : get_out_parameter_outputs(node))
                output_identifiers_[node->getName() + "." + output->getName()] = create_unique_identifier(make_identifier(without_prefix(output)));
        }
    }

    string GraphDecompiler::create_unique_identifier(const string& name)
    {
        string result = name;
        for (size_t i = 1; contains(used_identifiers_, result); ++i)
            result = name + std::to_string(i);
        used_identifiers_.insert(result);
        return result;
    }

    const string& GraphDecompiler::get_identifier(const mx::NodePtr& node) const
    {
        return identifiers_.at(node);
    }

    string GraphDecompiler::get_output_identifier(const mx::NodePtr& node, const string& output_name)
    {
        const string key = node->getName() + "." + output_name;
        if (not contains(output_identifiers_, key))
            output_identifiers_[key] = create_unique_identifier(make_identifier(remove_prefix(output_name)));
        return output_identifiers_.at(key);
    }

    string GraphDecompiler::get_variable_type(const mx::NodePtr& node) const
    {
        if (node->getType() != mx::MULTI_OUTPUT_TYPE_STRING)
            return get_type_alias(node->getType());
        return get_return_type(get_return_outputs(node));
    }

    vector<mx::OutputPtr> GraphDecompiler::get_return_outputs(const mx::NodePtr& node) const
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        const vector<mx::OutputPtr> outputs = node_def ? node_def->getActiveOutputs() : node->getOutputs();

        if (decompiler_.is_void_function(node_def))
            return {};

        vector<mx::OutputPtr> result;
        for (const mx::OutputPtr& output : outputs)
        {
            if (is_return_output(output))
                result.push_back(output);
        }
        return result;
    }

    vector<mx::OutputPtr> GraphDecompiler::get_out_parameter_outputs(const mx::NodePtr& node) const
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def == nullptr)
            return {};

        vector<mx::OutputPtr> result;
        for (const mx::OutputPtr& output : node_def->getActiveOutputs())
        {
            if (has_prefix(output, OUT_PARAMETER_PREFIX))
                result.push_back(output);
        }
        return result;
    }

    string GraphDecompiler::resolve_output_name(const mx::NodePtr& node, const string& output_name) const
    {
        if (not output_name.empty())
            return output_name;

        const mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def and node_def->getActiveOutputs().size() == 1)
            return node_def->getActiveOutputs().front()->getName();
        return output_name;
    }

    bool GraphDecompiler::is_ref_parameter(const mx::NodeDefPtr& node_def, const string& param_name) const
    {
        // ref parameters are both an input and an out parameter output
        return node_def and node_def->getActiveInput(param_name) and node_def->getActiveOutput(with_prefix(OUT_PARAMETER_PREFIX, param_name));
    }

    vector<SourceCode> GraphDecompiler::create_statements(const mx::NodePtr& node)
    {
        vector<SourceCode> result;
        SourceCode stmt;

        if (is_geomprop_definition(node))
        {
            const mx::InputPtr default_input = node->getInput("default");
            const optional<ExpressionCode> default_value = default_input ? create_port_expression(default_input) : std::nullopt;
            stmt = format_variable_definition("geomprop", get_variable_type(node), get_identifier(node), default_value);
        }
        else if (get_assignments(node))
        {
            stmt = create_assignment(node);
        }
        else if (not get_out_parameter_outputs(node).empty())
        {
            const mx::NodeDefPtr node_def = node->getNodeDef();
            for (const mx::OutputPtr& output : get_out_parameter_outputs(node))
            {
                const string param_name = without_prefix(output);
                if (not is_ref_parameter(node_def, param_name))
                    continue;

                const optional<ExpressionCode> value = create_input_expression(node, param_name);
                const string var_name = get_output_identifier(node, output->getName());
                result.push_back(format_variable_definition("mutable", get_type_alias(output->getType()), var_name, value));
            }

            const optional<ExpressionCode> call = create_function_call(node, /*with_out_arguments*/true);

            bool is_return_value_used = false;
            if (contains(uses_, node))
            {
                for (const NodeUse& use : uses_.at(node))
                {
                    if (not has_prefix(resolve_output_name(node, use.output_name), OUT_PARAMETER_PREFIX))
                        is_return_value_used = true;
                }
            }

            if (is_return_value_used and not get_return_outputs(node).empty())
                stmt = format_variable_definition("", get_variable_type(node), get_identifier(node), call);
            else
                stmt = format_expression_statement(*call);
        }
        else if ((is_generated_node_name(node->getName()) and get_use_count(node) == 0 and is_untyped_statement(node)) or get_return_outputs(node).empty())
        {
            // the return type of an expression statement is not known, e.g., `texcoord<vec2>();`
            TypedContext context{is_typed_context_, false};
            const optional<ExpressionCode> expr = create_node_expression(node);
            if (not expr)
                return result;

            // swizzles are only evaluated when they are used, so they are assigned to a variable, e.g., `vec2 a = v.yx;`
            if (expr->precedence == Precedence::Postfix)
                stmt = format_variable_definition("", get_variable_type(node), get_identifier(node), expr);
            else
                stmt = format_expression_statement(*expr);
        }
        else
        {
            // variables that are assigned to by functions, i.e., nonlocal variables, must be mutable
            const bool is_mutable = graph_->isA<mx::Document>() and decompiler_.is_assigned_by_function(get_identifier(node));
            stmt = format_variable_definition(is_mutable ? "mutable" : "", get_variable_type(node), get_identifier(node), create_node_expression(node));
        }

        result.push_back(add_attributes(get_user_attributes(node), stmt));
        return result;
    }

    SourceCode GraphDecompiler::create_assignment(const mx::NodePtr& node)
    {
        const VariableAssignments& assignments = *get_assignments(node);
        const string& variable = assignments.identifier;
        const mx::NodePtr previous = get_previous_value(node);

        // the first value of a variable declares it, e.g., `mutable float x = a;`
        if (previous == nullptr and not assignments.is_declared_outside)
        {
            const bool is_assigned_by_function = graph_->isA<mx::Document>() and decompiler_.is_assigned_by_function(variable);
            const bool is_mutable = assignments.nodes.size() > 1 or is_assigned_by_function;
            return format_variable_definition(is_mutable ? "mutable" : "", get_variable_type(node), variable, create_node_expression(node));
        }

        // `q.y = a;`
        if (contains(swizzle_assignments_, node))
        {
            const SwizzleAssignment& swizzle = swizzle_assignments_.at(node);
            const optional<ExpressionCode> value = swizzle.value_separate
                ? create_input_expression(swizzle.value_separate, "in")
                : create_input_expression(node, swizzle.value_input);
            if (value)
                return SourceCode::concat({variable + "." + swizzle.channels + " = ", value->code, ";"});
        }

        // `x += a;`, or `x++;` if a is one
        if (const optional<string> op = get_compound_operator(node))
        {
            const bool is_increment = (*op == "+" or *op == "-") and (node->getType() == "float" or node->getType() == "integer");
            if (is_increment and is_one(node->getInput("in2")))
                return variable + *op + *op + ";";
            if (const optional<ExpressionCode> rhs = create_operand_expression(node, "in2"))
                return SourceCode::concat({variable + " " + *op + "= ", rhs->code, ";"});
        }

        // `x = if (c) { a };`, whose else branch is the previous value of x
        const ScopedValue<mx::NodePtr> implied_else{implied_else_, is_if_expression(node) ? previous : nullptr};
        const optional<ExpressionCode> value = create_node_expression(node);
        return SourceCode::concat({variable + " = ", value ? value->code : "null", ";"});
    }

    optional<ExpressionCode> GraphDecompiler::create_port_expression(const mx::PortElementPtr& port)
    {
        if (not port->getNodeName().empty())
        {
            const mx::NodePtr node = graph_->getNode(port->getNodeName());
            if (node == nullptr)
                return std::nullopt;
            return create_output_expression(node, port->getOutputString());
        }

        if (not port->getNodeGraphString().empty())
            return create_node_graph_reference(port);

        if (const string& interface_name = port->getAttribute(mx::ValueElement::INTERFACE_NAME_ATTRIBUTE); not interface_name.empty())
        {
            // functions access nonlocal variables through inputs named nonlocal_in__<name>
            if (has_prefix(interface_name, NONLOCAL_IN_PREFIX))
                return format_identifier(remove_prefix(interface_name));
            return format_identifier(make_identifier(interface_name));
        }

        if (port->hasValue())
            return format_value(port);

        return std::nullopt;
    }

    optional<ExpressionCode> GraphDecompiler::create_output_expression(const mx::NodePtr& node, const string& connected_output_name)
    {
        const string output_name = resolve_output_name(node, connected_output_name);
        const bool is_multi_output = node->getType() == mx::MULTI_OUTPUT_TYPE_STRING;

        if (is_output_constant(node))
            return create_input_expression(node, "value");

        if (not is_statement(node))
            return create_node_expression(node);

        // out arguments and nonlocal variables that were assigned to by the function call
        if (has_prefix(output_name, OUT_PARAMETER_PREFIX))
            return format_identifier(get_output_identifier(node, output_name));
        if (has_prefix(output_name, NONLOCAL_OUT_PREFIX))
            return format_identifier(remove_prefix(output_name));

        ExpressionCode var = format_identifier(get_identifier(node));
        if (not is_multi_output or output_name.empty() or get_return_outputs(node).size() == 1)
            return var;
        return format_field_access(var, remove_prefix(output_name, RETURN_VALUE_PREFIX));
    }

    optional<ExpressionCode> GraphDecompiler::create_input_expression(const mx::NodePtr& node, const string& input_name)
    {
        if (const mx::InputPtr input = node->getInput(input_name))
        {
            if (optional<ExpressionCode> expr = create_port_expression(input))
                return expr;
        }

        // unconnected inputs use the default value of the node def
        if (const mx::NodeDefPtr node_def = node->getNodeDef())
        {
            if (const mx::InputPtr input = node_def->getActiveInput(input_name))
                return format_value(input);
        }

        return std::nullopt;
    }

    optional<ExpressionCode> GraphDecompiler::create_operand_expression(const mx::NodePtr& node, const string& input_name)
    {
        const mx::InputPtr input = node->getInput(input_name);
        const bool is_typed = is_typed_context_ and input and input->getType() == node->getType();
        TypedContext context{is_typed_context_, is_typed};
        return create_input_expression(node, input_name);
    }

    optional<ExpressionCode> GraphDecompiler::create_untyped_expression(const mx::NodePtr& node, const string& input_name)
    {
        TypedContext context{is_typed_context_, false};
        return create_input_expression(node, input_name);
    }

    string GraphDecompiler::get_template_type(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const
    {
        if (node_def == nullptr or decompiler_.is_document_node_def(node_def))
            return "";

        // library functions are templated by the postfix of their node def name, e.g., ND_noise2d_float
        const string type_name = string_utils::get_postfix(node_def->getName(), '_');
        if (not is_type_name(type_name))
            return "";

        return is_return_type_ambiguous(node, node_def) ? get_type_alias(type_name) : "";
    }

    bool GraphDecompiler::is_return_type_ambiguous(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const
    {
        if (node_def == nullptr)
            return false;

        // the overloads that accept the arguments of the call, which are only ambiguous if their return types differ
        unordered_set<string> return_types;
        for (const mx::NodeDefPtr& overload : decompiler_.document()->getMatchingNodeDefs(node_def->getNodeString()))
        {
            bool accepts_arguments = true;
            for (const mx::InputPtr& input : node->getInputs())
            {
                // filenames are written as string literals, which are also accepted by string parameters, e.g.,
                // `constant<filename>("a.png")`
                const mx::InputPtr param = overload->getActiveInput(input->getName());
                const bool is_string_literal = input->getType() == "filename" and not is_connected(input);
                if (param == nullptr or (param->getType() != input->getType() and not (is_string_literal and param->getType() == "string")))
                    accepts_arguments = false;
            }

            if (accepts_arguments)
                return_types.insert(overload->getType());
        }

        return return_types.size() > 1;
    }

    bool GraphDecompiler::is_untyped_statement(const mx::NodePtr& node) const
    {
        // struct values and the functions of the document that are overloaded by their return type are assigned to a
        // variable, whose type selects the overload, e.g., `vec2 v = foo();`, which cannot be done by a template type
        if (node->getType() == mx::MULTI_OUTPUT_TYPE_STRING)
            return false;
        const mx::NodeDefPtr node_def = node->getNodeDef();
        return not (decompiler_.is_document_node_def(node_def) and is_return_type_ambiguous(node, node_def));
    }

    bool GraphDecompiler::is_parameter_type_unique(const mx::NodeDefPtr& node_def, const string& param_name) const
    {
        unordered_set<string> param_types;
        for (const mx::NodeDefPtr& overload : decompiler_.document()->getMatchingNodeDefs(node_def->getNodeString()))
        {
            if (const mx::InputPtr param = overload->getActiveInput(param_name))
                param_types.insert(param->getType());
        }
        return param_types.size() <= 1;
    }

    optional<ExpressionCode> GraphDecompiler::create_node_expression(const mx::NodePtr& node)
    {
        const string& category = node->getCategory();

        if (category == "invert")
        {
            const mx::InputPtr amount = node->getInput("amount");
            const bool is_zero_amount = amount and amount->getNodeName().empty() and amount->hasValue() and is_zero_value(amount);
            if (is_zero_amount and is_connected(node->getInput("in")))
            {
                if (const optional<ExpressionCode> in = create_operand_expression(node, "in"))
                    return format_unary_expression("-", *in);
            }
        }

        if (category == "not")
        {
            // `a != b` is compiled to `not(ifequal(a, b))`
            const mx::NodePtr in_node = get_connected_node(node->getInput("in"));
            if (in_node and not is_statement(in_node) and in_node->getCategory() == "ifequal" and is_comparison(in_node))
            {
                const optional<ExpressionCode> lhs = create_untyped_expression(in_node, "value1");
                const optional<ExpressionCode> rhs = create_untyped_expression(in_node, "value2");
                if (lhs and rhs)
                    return format_binary_expression(*lhs, "!=", *rhs);
            }

            if (const optional<ExpressionCode> in = create_operand_expression(node, "in"))
                return format_unary_expression("!", *in);
        }

        if (category == "absval")
        {
            if (const optional<ExpressionCode> in = create_operand_expression(node, "in"))
                return format_absolute_value(*in);
        }

        if (contains(get_binary_operators(), category) and not is_integer_arithmetic(node))
        {
            if (optional<ExpressionCode> expr = create_binary_expression(node, get_binary_operators().at(category)))
                return expr;
        }

        if (is_comparison(node))
        {
            if (optional<ExpressionCode> expr = create_comparison_expression(node))
                return expr;
        }

        if (is_if_expression(node))
        {
            if (optional<ExpressionCode> expr = create_if_expression(node))
                return expr;
        }

        if (category == "extract")
        {
            if (optional<ExpressionCode> expr = create_extract_expression(node))
                return expr;
        }

        if (is_combine(node))
        {
            if (optional<ExpressionCode> expr = create_combine_expression(node))
                return expr;
        }

        if (category == "convert")
        {
            // `vec3{f}` is compiled to `convert(f)`, but constant arguments are evaluated at compile time
            if (is_connected(node->getInput("in")))
            {
                if (const optional<ExpressionCode> in = create_untyped_expression(node, "in"))
                    return format_constructor(get_type_alias(node->getType()), {*in});
            }
        }

        return create_function_call(node, /*with_out_arguments*/false);
    }

    optional<ExpressionCode> GraphDecompiler::create_binary_expression(const mx::NodePtr& node, const string& op)
    {
        optional<ExpressionCode> lhs;
        if (const mx::NodePtr convert = get_float_operand_convert(node))
            lhs = create_untyped_expression(convert, "in");
        else
            lhs = create_operand_expression(node, "in1");

        const optional<ExpressionCode> rhs = create_operand_expression(node, "in2");

        if (not lhs or not rhs)
            return std::nullopt;
        return format_binary_expression(*lhs, op, *rhs);
    }

    optional<ExpressionCode> GraphDecompiler::create_comparison_expression(const mx::NodePtr& node)
    {
        static const unordered_map<string, string> symbols {
            {"ifgreater", ">"},
            {"ifgreatereq", ">="},
            {"ifequal", "=="},
        };

        const optional<ExpressionCode> lhs = create_untyped_expression(node, "value1");
        const optional<ExpressionCode> rhs = create_untyped_expression(node, "value2");
        if (not lhs or not rhs)
            return std::nullopt;
        return format_binary_expression(*lhs, symbols.at(node->getCategory()), *rhs);
    }

    optional<ExpressionCode> GraphDecompiler::create_if_expression(const mx::NodePtr& node)
    {
        // only the last else branch can be implied, e.g., not the else branch of an if-expression in a then branch
        const mx::NodePtr implied_else = implied_else_;
        const ScopedValue<mx::NodePtr> context{implied_else_, nullptr};

        const optional<ExpressionCode> condition = create_untyped_expression(node, "value1");
        const optional<ExpressionCode> then_code = create_operand_expression(node, "in1");
        if (not condition or not then_code)
            return std::nullopt;

        const mx::NodePtr else_node = get_connected_node(node->getInput("in2"));
        if (implied_else and else_node == implied_else)
            return format_if_expression(*condition, *then_code, std::nullopt);

        // e.g., `x = if (a) { 1.0 } else if (b) { 2.0 };`
        if (else_node and not is_statement(else_node) and is_if_expression(else_node))
            implied_else_ = implied_else;

        const optional<ExpressionCode> else_code = create_operand_expression(node, "in2");
        if (not else_code)
            return std::nullopt;
        return format_if_expression(*condition, *then_code, *else_code);
    }

    optional<ExpressionCode> GraphDecompiler::create_extract_expression(const mx::NodePtr& node)
    {
        const mx::InputPtr in = node->getInput("in");
        if (in == nullptr)
            return std::nullopt;

        const optional<ExpressionCode> in_code = create_untyped_expression(node, "in");
        const optional<ExpressionCode> index_code = create_untyped_expression(node, "index");
        if (not in_code or not index_code)
            return std::nullopt;

        // `v.y` is compiled to `extract(v, 1)`
        const string channels = get_swizzle_channels(in->getType());
        const mx::InputPtr index_input = node->getInput("index");
        const optional<int> index = index_input ? get_int_value(index_input) : 0;

        if (index and *index >= 0 and static_cast<size_t>(*index) < channels.size())
            return format_member_access(*in_code, string{channels[*index]});

        // `v[i]` is compiled to `extract(v, i)`
        return format_indexing(*in_code, *index_code);
    }

    optional<ExpressionCode> GraphDecompiler::create_combine_expression(const mx::NodePtr& node)
    {
        const size_t count = get_channel_count(node);

        if (const mx::NodePtr separate = get_swizzle_separate(node))
        {
            const optional<ExpressionCode> value = create_untyped_expression(separate, "in");
            if (not value)
                return std::nullopt;

            string swizzle;
            for (size_t i = 1; i <= count; ++i)
                swizzle += *get_separate_channel(node->getInput("in" + std::to_string(i))->getOutputString());

            return format_member_access(*value, swizzle);
        }

        const vector<SeparatedArgument> arguments = get_separated_arguments(node);

        // constructors with only constant arguments are evaluated at compile time
        bool has_connected_input = not arguments.empty();
        for (size_t i = 1; i <= count; ++i)
            has_connected_input = has_connected_input or is_connected(node->getInput("in" + std::to_string(i)));
        if (not has_connected_input)
            return std::nullopt;

        vector<ExpressionCode> args;
        size_t i = 0;
        while (i < count)
        {
            optional<ExpressionCode> arg;
            const auto argument = std::find_if(arguments.begin(), arguments.end(), [i](const SeparatedArgument& a) { return a.start == i; });
            if (argument != arguments.end())
            {
                arg = create_untyped_expression(argument->separate, "in");
                i += argument->count;
            }
            else
            {
                arg = create_untyped_expression(node, "in" + std::to_string(i + 1));
                ++i;
            }

            if (not arg)
                return std::nullopt;
            args.push_back(*arg);
        }

        return format_constructor(get_type_alias(node->getType()), args);
    }

    optional<ExpressionCode> GraphDecompiler::create_function_call(const mx::NodePtr& node, const bool with_out_arguments)
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        const bool is_document_function = node_def and decompiler_.is_document_node_def(node_def);
        const string func_name = is_document_function ? decompiler_.get_function_name(node_def) : node->getCategory();
        const string func_template_type = is_typed_context_ ? "" : get_template_type(node, node_def);

        TypedContext context{is_typed_context_, true};
        vector<SourceCode> args;
        unordered_set<string> handled_inputs;

        // arguments are positional until an input is skipped, after which they must be named
        bool is_named = false;
        const auto add_argument = [&](const vector<string>& attrs, const string& name, const ExpressionCode& value) {
            args.push_back(format_argument(attrs, is_named ? name : "", value));
        };

        if (node_def)
        {
            for (const mx::InputPtr& param : node_def->getActiveInputs())
            {
                const string& name = param->getName();
                handled_inputs.insert(name);

                // implicit inputs, i.e., the nonlocal variables accessed by the function
                if (has_prefix(name, NONLOCAL_IN_PREFIX))
                    continue;

                const mx::InputPtr input = node->getInput(name);
                TypedContext param_context{is_typed_context_, is_parameter_type_unique(node_def, name)};
                optional<ExpressionCode> value = input ? create_port_expression(input) : std::nullopt;

                // ref arguments are variables declared before the function call
                if (with_out_arguments and is_ref_parameter(node_def, name))
                    value = format_identifier(get_output_identifier(node, with_prefix(OUT_PARAMETER_PREFIX, name)));
                if (not value and decompiler_.is_required_input(node_def, name))
                    value = format_value(param);

                if (not value)
                {
                    is_named = true;
                    continue;
                }

                add_argument(input ? get_user_attributes(input) : vector<string>{}, name, *value);
            }

            if (with_out_arguments)
            {
                for (const mx::OutputPtr& output : get_out_parameter_outputs(node))
                {
                    if (is_ref_parameter(node_def, without_prefix(output)))
                        continue;

                    // out arguments are declared by the call, e.g., `sincos(x, float s, float c);`
                    const ExpressionCode declaration = format_identifier(get_type_alias(output->getType()) + " " + get_output_identifier(node, output->getName()));
                    add_argument({}, without_prefix(output), declaration);
                }
            }
        }

        // inputs that are not part of the node def, e.g., if the node def is unknown
        is_named = true;
        for (const mx::InputPtr& input : node->getInputs())
        {
            if (contains(handled_inputs, input->getName()))
                continue;
            if (const optional<ExpressionCode> value = create_port_expression(input))
                add_argument(get_user_attributes(input), input->getName(), *value);
        }

        return format_function_call(func_name, func_template_type, args);
    }

    optional<ExpressionCode> GraphDecompiler::create_node_graph_reference(const mx::PortElementPtr& port) const
    {
        const mx::NodeGraphPtr node_graph = decompiler_.document()->getNodeGraph(port->getNodeGraphString());
        if (node_graph == nullptr)
            return std::nullopt;

        const string func_name = decompiler_.get_function_name(node_graph);

        // parameterless functions are referenced without an argument list
        ExpressionCode function = node_graph->getInputs().empty() ? format_identifier(func_name) : format_function_call(func_name, "", {});

        const string& output_name = port->getOutputString();
        if (node_graph->getOutputs().size() > 1 and not output_name.empty())
            return format_field_access(function, remove_prefix(output_name, RETURN_VALUE_PREFIX));
        return function;
    }
}
