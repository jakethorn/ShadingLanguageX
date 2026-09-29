//
// Created by jaket on 28/09/2026.
//

#include "decompile/GraphDecompiler.h"

#include <functional>

#include "decompile/DocumentDecompiler.h"
#include "decompile/decompile_utils.h"
#include "serialize/serialize_name_utils.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace decompile_utils;
    using container_utils::contains;

    namespace
    {
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

        // the number of inputs of a combine node or outputs of a separate node
        size_t channel_count(const mx::NodePtr& node)
        {
            return static_cast<size_t>(node->getCategory().back() - '0');
        }

        bool is_integer_arithmetic(const mx::NodePtr& node)
        {
            // integer multiply, divide, modulo and power are not compiled to a single node, see stdlib.mxsl
            const string& category = node->getCategory();
            const bool is_int_op = category == "multiply" or category == "divide" or category == "modulo" or category == "power";
            return is_int_op and node->getType() == "integer";
        }

        const unordered_map<string, string>& binary_operators()
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

        bool has_bool_value(const mx::InputPtr& input, const bool expected)
        {
            if (input == nullptr or not input->getNodeName().empty() or not input->hasValue())
                return false;
            const mx::ValuePtr value = input->getValue();
            return value and value->isA<bool>() and value->asA<bool>() == expected;
        }

        optional<int> int_value(const mx::InputPtr& input)
        {
            if (input == nullptr or not input->getNodeName().empty() or not input->getInterfaceName().empty() or not input->hasValue())
                return std::nullopt;
            const mx::ValuePtr value = input->getValue();
            if (value and value->isA<int>())
                return value->asA<int>();
            return std::nullopt;
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

        string output_field_name(const string& output_name)
        {
            // outputs of functions that return a struct are named out__<field>
            if (serialize::has_prefix(output_name, serialize::RETURN_VALUE_PREFIX))
                return serialize::remove_prefix(output_name);
            return output_name;
        }

        // a field of a struct, which is indexed if the struct has no field names, e.g., `f[0]`
        Code field_access(const Code& value, const string& field_name)
        {
            if (is_index(field_name))
                return code::index(value, code::literal(field_name));
            return code::member(value, to_identifier(field_name));
        }

        class TypedContext
        {
        public:
            TypedContext(bool& flag, const bool value) : flag_{flag}, saved_{flag} { flag_ = value; }
            ~TypedContext() { flag_ = saved_; }
            TypedContext(const TypedContext&) = delete;
            TypedContext& operator=(const TypedContext&) = delete;

        private:
            bool& flag_;
            bool saved_;
        };

        bool is_type_name(const string& name)
        {
            static const unordered_set<string> type_names {
                "boolean", "integer", "float", "vector2", "vector3", "vector4", "color3", "color4", "matrix33", "matrix44",
                "string", "filename", "surfaceshader", "displacementshader", "volumeshader", "lightshader", "material",
                "BSDF", "EDF", "VDF"
            };
            return contains(type_names, name);
        }

        string variable_definition(const string& mods, const string& type, const string& name, const optional<Code>& value)
        {
            string result = mods.empty() ? "" : mods + " ";
            result += type + " " + name;
            if (value)
                result += " = " + value->text;
            return result + ";";
        }

        string expression_statement(const Code& expr)
        {
            // statements beginning with `if` are if statements, so if expressions are wrapped in parentheses
            const bool is_if_expr = expr.precedence == Precedence::Lowest and string_utils::starts_with(expr.text, "if (");
            return (is_if_expr ? "(" + expr.text + ")" : expr.text) + ";";
        }
    }

    GraphDecompiler::GraphDecompiler(DocumentDecompiler& document, mx::GraphElementPtr graph, const unordered_set<string>& reserved_names)
        : document_{document}, graph_{std::move(graph)}, nodes_{graph_->getNodes()}
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

        find_absorbed_nodes();
        find_statements();
        create_identifiers(reserved_names);
    }

    bool GraphDecompiler::is_statement(const mx::NodePtr& node) const
    {
        return contains(statements_, node);
    }

    size_t GraphDecompiler::use_count(const mx::NodePtr& node) const
    {
        if (contains(uses_, node))
            return uses_.at(node).size();
        return 0;
    }

    vector<mx::NodePtr> GraphDecompiler::ordered_statements() const
    {
        vector<mx::NodePtr> result;
        unordered_set<mx::NodePtr> visited;

        std::function<void(const mx::NodePtr&)> visit = [&](const mx::NodePtr& node) {
            if (contains(visited, node))
                return;
            visited.insert(node);
            for (const mx::NodePtr& dependency : statement_dependencies(node))
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

    vector<mx::NodePtr> GraphDecompiler::statement_dependencies(const mx::NodePtr& node) const
    {
        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited{node};
        for (const mx::InputPtr& input : node->getInputs())
            collect_dependencies(input, statements, functions, visited);
        return statements;
    }

    vector<mx::ElementPtr> GraphDecompiler::function_dependencies(const mx::NodePtr& node) const
    {
        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited{node};
        collect_dependencies(node, statements, functions, visited);
        return functions;
    }

    vector<mx::ElementPtr> GraphDecompiler::function_dependencies(const mx::PortElementPtr& port) const
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
        if (node_def and document_.is_document_node_def(node_def) and not contains(functions, node_def))
            functions.push_back(node_def);

        for (const mx::InputPtr& input : node->getInputs())
            collect_dependencies(input, statements, functions, visited);
    }

    void GraphDecompiler::collect_dependencies(const mx::PortElementPtr& port, vector<mx::NodePtr>& statements, vector<mx::ElementPtr>& functions, unordered_set<mx::NodePtr>& visited) const
    {
        if (const string& node_graph_name = port->getNodeGraphString(); not node_graph_name.empty())
        {
            const mx::NodeGraphPtr node_graph = document_.document()->getNodeGraph(node_graph_name);
            if (node_graph and not contains(functions, node_graph))
                functions.push_back(node_graph);
        }

        const mx::NodePtr node = port->isA<mx::Input>() ? connected_node(port->asA<mx::Input>()) : graph_->getNode(port->getNodeName());
        if (node == nullptr or contains(visited, node))
            return;

        if (is_statement(node))
        {
            if (not contains(statements, node))
                statements.push_back(node);
            // statements are declared separately, but their functions are still dependencies of this expression
            const mx::NodeDefPtr node_def = node->getNodeDef();
            if (node_def and document_.is_document_node_def(node_def) and not contains(functions, node_def))
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

        uses_[node].push_back(Use{consumer, port->getOutputString()});
    }

    bool GraphDecompiler::is_absorbable_by(const mx::NodePtr& helper, const mx::NodePtr& consumer, const size_t expected_uses) const
    {
        if (helper == nullptr or helper == consumer or not contains(uses_, helper))
            return false;

        const vector<Use>& uses = uses_.at(helper);
        if (uses.size() != expected_uses)
            return false;

        for (const Use& use : uses)
        {
            if (use.consumer != consumer)
                return false;
        }

        return user_attributes(helper).empty();
    }

    mx::NodePtr GraphDecompiler::connected_node(const mx::InputPtr& input) const
    {
        if (input == nullptr or input->getNodeName().empty())
            return nullptr;
        return graph_->getNode(input->getNodeName());
    }

    // `1.0 - v` is compiled to `subtract(convert(1.0), v)`
    mx::NodePtr GraphDecompiler::float_first_convert(const mx::NodePtr& node) const
    {
        const string& category = node->getCategory();
        if (category != "subtract" and category != "divide" and category != "modulo" and category != "power")
            return nullptr;

        const mx::NodePtr convert = connected_node(node->getInput("in1"));
        if (convert == nullptr or convert->getCategory() != "convert" or convert->getType() != node->getType())
            return nullptr;

        const mx::InputPtr convert_input = convert->getInput("in");
        if (convert_input == nullptr or convert_input->getType() != "float")
            return nullptr;

        return is_absorbable_by(convert, node, 1) ? convert : nullptr;
    }

    // `v.zx` is compiled to `combine2(separate3(v).outz, separate3(v).outx)`, v can also be a value, e.g., `vec3{}.zx`
    mx::NodePtr GraphDecompiler::swizzle_separate(const mx::NodePtr& node) const
    {
        if (not is_combine(node) or (node->getType() == "vector4" and node->getCategory() == "combine2"))
            return nullptr;

        const bool is_color = is_color_type(node->getType());
        const size_t count = channel_count(node);

        mx::NodePtr separate;
        for (size_t i = 1; i <= count; ++i)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i));
            const mx::NodePtr input_node = connected_node(input);
            if (input_node == nullptr or not is_separate(input_node))
                return nullptr;
            if (separate and input_node != separate)
                return nullptr;
            separate = input_node;
            if (separate->getInput("in") == nullptr)
                return nullptr;

            const optional<char> channel = swizzle_channel(input->getOutputString());
            if (not channel)
                return nullptr;
            const bool is_color_channel = string{"rgba"}.find(*channel) != string::npos;
            if (is_color_channel != is_color)
                return nullptr;
        }

        return is_absorbable_by(separate, node, count) ? separate : nullptr;
    }

    // `vec3{uv, 1.0}` is compiled to `combine3(separate2(uv).outx, separate2(uv).outy, 1.0)`
    vector<GraphDecompiler::ConstructorRun> GraphDecompiler::constructor_runs(const mx::NodePtr& node) const
    {
        if (node->getCategory() != "combine3" and node->getCategory() != "combine4")
            return {};

        const size_t count = channel_count(node);

        vector<ConstructorRun> runs;
        vector<string> arg_types;
        size_t i = 0;
        while (i < count)
        {
            const mx::InputPtr input = node->getInput("in" + std::to_string(i + 1));
            const mx::NodePtr separate = connected_node(input);

            bool is_run = separate and is_separate(separate) and i + channel_count(separate) <= count;
            const size_t run_count = is_run ? channel_count(separate) : 0;
            for (size_t j = 0; is_run and j < run_count; ++j)
            {
                const mx::InputPtr run_input = node->getInput("in" + std::to_string(i + j + 1));
                const optional<char> channel = swizzle_channel(run_input->getOutputString());
                const bool is_in_order = channel and (*channel == "xyzw"[j] or *channel == "rgba"[j]);
                is_run = connected_node(run_input) == separate and is_in_order;
            }
            is_run = is_run and is_absorbable_by(separate, node, run_count);

            if (is_run)
            {
                const mx::InputPtr separate_input = separate->getInput("in");
                if (separate_input == nullptr)
                    return {};
                runs.push_back(ConstructorRun{i, run_count, separate});
                arg_types.push_back(separate_input->getType());
                i += run_count;
            }
            else
            {
                arg_types.emplace_back("float");
                ++i;
            }
        }

        if (runs.empty() or not is_constructor_signature(node->getType(), arg_types))
            return {};
        return runs;
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
        if (node->getCategory() != "geompropvalue" or is_temporary_name(node->getName()) or not is_valid_identifier(node->getName()))
            return false;

        const mx::InputPtr geomprop = node->getInput("geomprop");
        if (geomprop == nullptr or not geomprop->getNodeName().empty() or not geomprop->hasValue() or geomprop->getValueString() != node->getName())
            return false;

        for (const mx::InputPtr& input : node->getInputs())
        {
            if (input->getName() != "geomprop" and input->getName() != "default")
                return false;
        }

        return user_attributes(geomprop).empty();
    }

    // `return 1.0;` is compiled to an output connected to `constant(1.0)`
    bool GraphDecompiler::is_output_constant(const mx::NodePtr& node) const
    {
        if (not graph_->isA<mx::NodeGraph>() or node->getCategory() != "constant" or use_count(node) != 1)
            return false;
        if (not uses_.at(node).front().consumer->isA<mx::Output>())
            return false;

        const mx::InputPtr value = node->getInput("value");
        if (value == nullptr or is_connected(value) or not value->hasValue() or not has_literal_syntax(value->getType()))
            return false;

        return user_attributes(node).empty() and user_attributes(value).empty() and node->getInputs().size() == 1;
    }

    void GraphDecompiler::find_absorbed_nodes()
    {
        for (const mx::NodePtr& node : nodes_)
        {
            if (const mx::NodePtr convert = float_first_convert(node))
                absorbed_.insert(convert);

            if (const mx::NodePtr separate = swizzle_separate(node))
            {
                absorbed_.insert(separate);
            }
            else
            {
                for (const ConstructorRun& run : constructor_runs(node))
                    absorbed_.insert(run.separate);
            }
        }
    }

    void GraphDecompiler::find_statements()
    {
        for (const mx::NodePtr& node : nodes_)
        {
            if (contains(absorbed_, node) or is_output_constant(node))
                continue;

            const bool is_named = not is_temporary_name(node->getName());
            const bool is_shared = use_count(node) != 1;
            const bool has_attributes = not user_attributes(node).empty();
            const bool has_multiple_outputs = node->getType() == mx::MULTI_OUTPUT_TYPE_STRING;
            const bool has_out_parameters = not out_parameter_outputs(node).empty();
            // e.g., void functions that assign to nonlocal variables
            const bool has_no_return_value = return_outputs(node).empty();

            if (is_named or is_shared or has_attributes or has_multiple_outputs or has_out_parameters or has_no_return_value)
                statements_.insert(node);
        }
    }

    void GraphDecompiler::create_identifiers(const unordered_set<string>& reserved_names)
    {
        used_identifiers_ = reserved_names;

        // named nodes take priority over the names generated for temporaries
        for (const mx::NodePtr& node : nodes_)
        {
            if (not is_temporary_name(node->getName()))
                identifiers_[node] = unique_identifier(to_identifier(node->getName()));
        }

        for (const mx::NodePtr& node : nodes_)
        {
            if (is_temporary_name(node->getName()))
                identifiers_[node] = unique_identifier("var_" + node->getName().substr(5));
        }

        // out arguments are declared as part of the function call, e.g., `sincos(x, float s, float c);`
        for (const mx::NodePtr& node : nodes_)
        {
            for (const mx::OutputPtr& output : out_parameter_outputs(node))
                output_identifiers_[node->getName() + "." + output->getName()] = unique_identifier(to_identifier(serialize::remove_prefix(output->getName())));
        }
    }

    string GraphDecompiler::unique_identifier(const string& name)
    {
        string result = name;
        for (size_t i = 1; contains(used_identifiers_, result); ++i)
            result = name + std::to_string(i);
        used_identifiers_.insert(result);
        return result;
    }

    const string& GraphDecompiler::identifier(const mx::NodePtr& node) const
    {
        return identifiers_.at(node);
    }

    string GraphDecompiler::output_identifier(const mx::NodePtr& node, const string& output_name)
    {
        const string key = node->getName() + "." + output_name;
        if (not contains(output_identifiers_, key))
            output_identifiers_[key] = unique_identifier(to_identifier(serialize::remove_prefix(output_name)));
        return output_identifiers_.at(key);
    }

    string GraphDecompiler::node_type(const mx::NodePtr& node) const
    {
        const vector<mx::OutputPtr> outputs = return_outputs(node);

        if (node->getType() != mx::MULTI_OUTPUT_TYPE_STRING)
            return type_alias(node->getType());
        if (outputs.size() == 1)
            return type_alias(outputs.front()->getType());

        string fields;
        for (const mx::OutputPtr& output : outputs)
        {
            const string field_name = output_field_name(output->getName());
            fields += (fields.empty() ? "" : ", ") + type_alias(output->getType());
            if (not is_index(field_name))
                fields += " " + to_identifier(field_name);
        }
        return "{" + fields + "}";
    }

    vector<mx::OutputPtr> GraphDecompiler::return_outputs(const mx::NodePtr& node) const
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        const vector<mx::OutputPtr> outputs = node_def ? node_def->getActiveOutputs() : node->getOutputs();

        if (document_.is_void_function(node_def))
            return {};

        vector<mx::OutputPtr> result;
        for (const mx::OutputPtr& output : outputs)
        {
            const string& name = output->getName();
            if (not serialize::has_prefix(name, serialize::OUT_PARAMETER_PREFIX) and not serialize::has_prefix(name, serialize::NONLOCAL_OUT_PREFIX))
                result.push_back(output);
        }
        return result;
    }

    vector<mx::OutputPtr> GraphDecompiler::out_parameter_outputs(const mx::NodePtr& node) const
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def == nullptr)
            return {};

        vector<mx::OutputPtr> result;
        for (const mx::OutputPtr& output : node_def->getActiveOutputs())
        {
            if (serialize::has_prefix(output->getName(), serialize::OUT_PARAMETER_PREFIX))
                result.push_back(output);
        }
        return result;
    }

    string GraphDecompiler::resolved_output_name(const mx::NodePtr& node, const string& output_name) const
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
        return node_def and node_def->getActiveInput(param_name) and node_def->getActiveOutput(serialize::with_prefix(serialize::OUT_PARAMETER_PREFIX, param_name));
    }

    vector<string> GraphDecompiler::create_statements(const mx::NodePtr& node)
    {
        vector<string> result;
        string stmt;

        if (is_geomprop_definition(node))
        {
            const mx::InputPtr default_input = node->getInput("default");
            const optional<Code> default_value = default_input ? create_port_expression(default_input) : std::nullopt;
            stmt = variable_definition("geomprop", node_type(node), identifier(node), default_value);
        }
        else if (not out_parameter_outputs(node).empty())
        {
            const mx::NodeDefPtr node_def = node->getNodeDef();
            for (const mx::OutputPtr& output : out_parameter_outputs(node))
            {
                const string param_name = serialize::remove_prefix(output->getName());
                if (not is_ref_parameter(node_def, param_name))
                    continue;

                const optional<Code> value = create_input_expression(node, param_name);
                const string var_name = output_identifier(node, output->getName());
                result.push_back(variable_definition("mutable", type_alias(output->getType()), var_name, value));
            }

            const optional<Code> call = create_function_call(node, /*with_out_arguments*/true);

            bool is_return_value_used = false;
            if (contains(uses_, node))
            {
                for (const Use& use : uses_.at(node))
                {
                    if (not serialize::has_prefix(resolved_output_name(node, use.output_name), serialize::OUT_PARAMETER_PREFIX))
                        is_return_value_used = true;
                }
            }

            if (is_return_value_used and not return_outputs(node).empty())
                stmt = variable_definition("", node_type(node), identifier(node), call);
            else
                stmt = expression_statement(*call);
        }
        else if ((is_temporary_name(node->getName()) and use_count(node) == 0 and is_untyped_statement(node)) or return_outputs(node).empty())
        {
            // the return type of an expression statement is not known, e.g., `texcoord<vec2>();`
            TypedContext context{is_typed_context_, false};
            const optional<Code> expr = create_node_expression(node);
            if (not expr)
                return result;

            // swizzles are only evaluated when they are used, so they are assigned to a variable, e.g., `vec2 a = v.yx;`
            if (expr->precedence == Precedence::Postfix)
                stmt = variable_definition("", node_type(node), identifier(node), expr);
            else
                stmt = expression_statement(*expr);
        }
        else
        {
            // variables that are assigned to by functions, i.e., nonlocal variables, must be mutable
            const bool is_mutable = graph_->isA<mx::Document>() and document_.is_mutable_variable(identifier(node));
            stmt = variable_definition(is_mutable ? "mutable" : "", node_type(node), identifier(node), create_node_expression(node));
        }

        result.push_back(code::with_attributes(user_attributes(node), stmt));
        return result;
    }

    optional<Code> GraphDecompiler::create_port_expression(const mx::PortElementPtr& port)
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
            if (serialize::has_prefix(interface_name, serialize::NONLOCAL_IN_PREFIX))
                return code::identifier(serialize::remove_prefix(interface_name));
            return code::identifier(to_identifier(interface_name));
        }

        if (port->hasValue())
            return literal(port);

        return std::nullopt;
    }

    optional<Code> GraphDecompiler::create_output_expression(const mx::NodePtr& node, const string& connected_output_name)
    {
        const string output_name = resolved_output_name(node, connected_output_name);
        const bool is_multi_output = node->getType() == mx::MULTI_OUTPUT_TYPE_STRING;

        if (is_output_constant(node))
            return create_input_expression(node, "value");

        if (not is_statement(node))
            return create_node_expression(node);

        // out arguments and nonlocal variables that were assigned to by the function call
        if (serialize::has_prefix(output_name, serialize::OUT_PARAMETER_PREFIX))
            return code::identifier(output_identifier(node, output_name));
        if (serialize::has_prefix(output_name, serialize::NONLOCAL_OUT_PREFIX))
            return code::identifier(serialize::remove_prefix(output_name));

        const Code var = code::identifier(identifier(node));
        if (not is_multi_output or output_name.empty() or return_outputs(node).size() == 1)
            return var;
        return field_access(var, output_field_name(output_name));
    }

    optional<Code> GraphDecompiler::create_input_expression(const mx::NodePtr& node, const string& input_name)
    {
        if (const mx::InputPtr input = node->getInput(input_name))
        {
            if (optional<Code> expr = create_port_expression(input))
                return expr;
        }

        // unconnected inputs use the default value of the node def
        if (const mx::NodeDefPtr node_def = node->getNodeDef())
        {
            if (const mx::InputPtr input = node_def->getActiveInput(input_name))
                return literal(input);
        }

        return std::nullopt;
    }

    optional<Code> GraphDecompiler::create_operand_expression(const mx::NodePtr& node, const string& input_name, const mx::NodePtr& operation)
    {
        const mx::InputPtr input = node->getInput(input_name);
        const bool is_typed = is_typed_context_ and input and input->getType() == operation->getType();
        TypedContext context{is_typed_context_, is_typed};
        return create_input_expression(node, input_name);
    }

    optional<Code> GraphDecompiler::create_untyped_expression(const mx::NodePtr& node, const string& input_name)
    {
        TypedContext context{is_typed_context_, false};
        return create_input_expression(node, input_name);
    }

    string GraphDecompiler::template_type(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const
    {
        if (node_def == nullptr or document_.is_document_node_def(node_def))
            return "";

        // library functions are templated by the postfix of their node def name, e.g., ND_noise2d_float
        const string type_name = string_utils::get_postfix(node_def->getName(), '_');
        if (not is_type_name(type_name))
            return "";

        return is_return_type_ambiguous(node, node_def) ? type_alias(type_name) : "";
    }

    bool GraphDecompiler::is_return_type_ambiguous(const mx::NodePtr& node, const mx::NodeDefPtr& node_def) const
    {
        if (node_def == nullptr)
            return false;

        // the overloads that accept the arguments of the call, which are only ambiguous if their return types differ
        unordered_set<string> return_types;
        for (const mx::NodeDefPtr& overload : document_.document()->getMatchingNodeDefs(node_def->getNodeString()))
        {
            bool accepts_arguments = true;
            for (const mx::InputPtr& input : node->getInputs())
            {
                const mx::InputPtr param = overload->getActiveInput(input->getName());
                if (param == nullptr or param->getType() != input->getType())
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
        return not (document_.is_document_node_def(node_def) and is_return_type_ambiguous(node, node_def));
    }

    bool GraphDecompiler::is_parameter_type_unique(const mx::NodeDefPtr& node_def, const string& param_name) const
    {
        unordered_set<string> param_types;
        for (const mx::NodeDefPtr& overload : document_.document()->getMatchingNodeDefs(node_def->getNodeString()))
        {
            if (const mx::InputPtr param = overload->getActiveInput(param_name))
                param_types.insert(param->getType());
        }
        return param_types.size() <= 1;
    }

    optional<Code> GraphDecompiler::create_node_expression(const mx::NodePtr& node)
    {
        const string& category = node->getCategory();

        if (category == "invert")
        {
            const mx::InputPtr amount = node->getInput("amount");
            const bool is_zero_amount = amount and amount->getNodeName().empty() and amount->hasValue() and is_zero_value(amount);
            if (is_zero_amount and is_connected(node->getInput("in")))
            {
                if (const optional<Code> in = create_operand_expression(node, "in", node))
                    return code::unary("-", *in);
            }
        }

        if (category == "not")
        {
            // `a != b` is compiled to `not(ifequal(a, b))`
            const mx::NodePtr in_node = connected_node(node->getInput("in"));
            if (in_node and not is_statement(in_node) and in_node->getCategory() == "ifequal" and is_comparison(in_node))
            {
                const optional<Code> lhs = create_untyped_expression(in_node, "value1");
                const optional<Code> rhs = create_untyped_expression(in_node, "value2");
                if (lhs and rhs)
                    return code::binary(*lhs, "!=", *rhs);
            }

            if (const optional<Code> in = create_operand_expression(node, "in", node))
                return code::unary("!", *in);
        }

        if (category == "absval")
        {
            if (const optional<Code> in = create_operand_expression(node, "in", node))
                return code::absolute(*in);
        }

        if (contains(binary_operators(), category) and not is_integer_arithmetic(node))
        {
            if (optional<Code> expr = create_binary_expression(node, binary_operators().at(category)))
                return expr;
        }

        if (is_comparison(node))
        {
            if (optional<Code> expr = create_comparison_expression(node))
                return expr;
        }

        if (is_if_expression(node))
        {
            if (optional<Code> expr = create_if_expression(node))
                return expr;
        }

        if (category == "extract")
        {
            if (optional<Code> expr = create_extract_expression(node))
                return expr;
        }

        if (is_combine(node))
        {
            if (optional<Code> expr = create_combine_expression(node))
                return expr;
        }

        if (category == "convert")
        {
            // `vec3{f}` is compiled to `convert(f)`, but constant arguments are evaluated at compile time
            if (is_connected(node->getInput("in")))
            {
                if (const optional<Code> in = create_untyped_expression(node, "in"))
                    return code::construct(type_alias(node->getType()), {*in});
            }
        }

        return create_function_call(node, /*with_out_arguments*/false);
    }

    optional<Code> GraphDecompiler::create_binary_expression(const mx::NodePtr& node, const string& op)
    {
        optional<Code> lhs;
        if (const mx::NodePtr convert = float_first_convert(node))
            lhs = create_untyped_expression(convert, "in");
        else
            lhs = create_operand_expression(node, "in1", node);

        const optional<Code> rhs = create_operand_expression(node, "in2", node);

        if (not lhs or not rhs)
            return std::nullopt;
        return code::binary(*lhs, op, *rhs);
    }

    optional<Code> GraphDecompiler::create_comparison_expression(const mx::NodePtr& node)
    {
        static const unordered_map<string, string> symbols {
            {"ifgreater", ">"},
            {"ifgreatereq", ">="},
            {"ifequal", "=="},
        };

        const optional<Code> lhs = create_untyped_expression(node, "value1");
        const optional<Code> rhs = create_untyped_expression(node, "value2");
        if (not lhs or not rhs)
            return std::nullopt;
        return code::binary(*lhs, symbols.at(node->getCategory()), *rhs);
    }

    optional<Code> GraphDecompiler::create_if_expression(const mx::NodePtr& node)
    {
        const optional<Code> condition = create_untyped_expression(node, "value1");
        const optional<Code> then_code = create_operand_expression(node, "in1", node);
        const optional<Code> else_code = create_operand_expression(node, "in2", node);
        if (not condition or not then_code or not else_code)
            return std::nullopt;
        return code::if_expression(*condition, *then_code, *else_code);
    }

    optional<Code> GraphDecompiler::create_extract_expression(const mx::NodePtr& node)
    {
        const mx::InputPtr in = node->getInput("in");
        if (in == nullptr)
            return std::nullopt;

        const optional<Code> in_code = create_untyped_expression(node, "in");
        const optional<Code> index_code = create_untyped_expression(node, "index");
        if (not in_code or not index_code)
            return std::nullopt;

        // `v.y` is compiled to `extract(v, 1)`
        const string& in_type = in->getType();
        const bool is_vector = in_type == "vector2" or in_type == "vector3" or in_type == "vector4";
        const bool is_color = is_color_type(in_type);
        const mx::InputPtr index_input = node->getInput("index");
        const optional<int> index = index_input ? int_value(index_input) : 0;

        if ((is_vector or is_color) and index and *index >= 0 and *index < 4)
        {
            const char channel = (is_color ? "rgba" : "xyzw")[*index];
            return code::member(*in_code, string{channel});
        }

        // `v[i]` is compiled to `extract(v, i)`
        return code::index(*in_code, *index_code);
    }

    optional<Code> GraphDecompiler::create_combine_expression(const mx::NodePtr& node)
    {
        const size_t count = channel_count(node);

        if (const mx::NodePtr separate = swizzle_separate(node))
        {
            const optional<Code> value = create_untyped_expression(separate, "in");
            if (not value)
                return std::nullopt;

            string swizzle;
            for (size_t i = 1; i <= count; ++i)
                swizzle += *swizzle_channel(node->getInput("in" + std::to_string(i))->getOutputString());

            return code::member(*value, swizzle);
        }

        const vector<ConstructorRun> runs = constructor_runs(node);

        // constructors with only constant arguments are evaluated at compile time
        bool has_connected_input = not runs.empty();
        for (size_t i = 1; i <= count; ++i)
            has_connected_input = has_connected_input or is_connected(node->getInput("in" + std::to_string(i)));
        if (not has_connected_input)
            return std::nullopt;

        vector<Code> args;
        size_t i = 0;
        while (i < count)
        {
            optional<Code> arg;
            const auto run = std::find_if(runs.begin(), runs.end(), [i](const ConstructorRun& r) { return r.start == i; });
            if (run != runs.end())
            {
                arg = create_untyped_expression(run->separate, "in");
                i += run->count;
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

        return code::construct(type_alias(node->getType()), args);
    }

    optional<Code> GraphDecompiler::create_function_call(const mx::NodePtr& node, const bool with_out_arguments)
    {
        const mx::NodeDefPtr node_def = node->getNodeDef();
        const bool is_document_function = node_def and document_.is_document_node_def(node_def);
        const string func_name = is_document_function ? document_.function_name(node_def) : node->getCategory();
        const string func_template_type = is_typed_context_ ? "" : template_type(node, node_def);

        TypedContext context{is_typed_context_, true};
        vector<string> args;
        unordered_set<string> handled_inputs;

        // arguments are positional until an input is skipped, after which they must be named
        bool is_named = false;
        const auto add_argument = [&](const vector<string>& attrs, const string& name, const Code& value) {
            args.push_back(code::argument(attrs, is_named ? name : "", value));
        };

        if (node_def)
        {
            for (const mx::InputPtr& param : node_def->getActiveInputs())
            {
                const string& name = param->getName();
                handled_inputs.insert(name);

                // implicit inputs, i.e., the nonlocal variables accessed by the function
                if (serialize::has_prefix(name, serialize::NONLOCAL_IN_PREFIX))
                    continue;

                const mx::InputPtr input = node->getInput(name);
                TypedContext param_context{is_typed_context_, is_parameter_type_unique(node_def, name)};
                optional<Code> value = input ? create_port_expression(input) : std::nullopt;

                // ref arguments are variables declared before the function call
                if (with_out_arguments and is_ref_parameter(node_def, name))
                    value = code::identifier(output_identifier(node, serialize::with_prefix(serialize::OUT_PARAMETER_PREFIX, name)));
                if (not value and document_.is_required_input(node_def, name))
                    value = literal(param);

                if (not value)
                {
                    is_named = true;
                    continue;
                }

                add_argument(input ? user_attributes(input) : vector<string>{}, name, *value);
            }

            if (with_out_arguments)
            {
                for (const mx::OutputPtr& output : out_parameter_outputs(node))
                {
                    if (is_ref_parameter(node_def, serialize::remove_prefix(output->getName())))
                        continue;

                    // out arguments are declared by the call, e.g., `sincos(x, float s, float c);`
                    const Code declaration = code::identifier(type_alias(output->getType()) + " " + output_identifier(node, output->getName()));
                    add_argument({}, serialize::remove_prefix(output->getName()), declaration);
                }
            }
        }

        // inputs that are not part of the node def, e.g., if the node def is unknown
        is_named = true;
        for (const mx::InputPtr& input : node->getInputs())
        {
            if (contains(handled_inputs, input->getName()))
                continue;
            if (const optional<Code> value = create_port_expression(input))
                add_argument(user_attributes(input), input->getName(), *value);
        }

        return code::call(func_name, func_template_type, args);
    }

    optional<Code> GraphDecompiler::create_node_graph_reference(const mx::PortElementPtr& port)
    {
        const mx::NodeGraphPtr node_graph = document_.document()->getNodeGraph(port->getNodeGraphString());
        if (node_graph == nullptr)
            return std::nullopt;

        const string func_name = document_.function_name(node_graph);

        // parameterless functions are referenced without an argument list
        const Code function = node_graph->getInputs().empty() ? code::identifier(func_name) : code::call(func_name, "", {});

        const string& output_name = port->getOutputString();
        if (node_graph->getOutputs().size() > 1 and not output_name.empty())
            return field_access(function, output_field_name(output_name));
        return function;
    }
}
