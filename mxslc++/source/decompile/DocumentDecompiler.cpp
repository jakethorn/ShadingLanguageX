//
// Created by jaket on 28/09/2026.
//

#include "decompile/DocumentDecompiler.h"

#include "decompile/decompile_utils.h"
#include "serialize/serialize_name_utils.h"
#include "utils/container_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace decompile_utils;
    using container_utils::contains;
    using string_utils::starts_with;

    namespace
    {
        bool is_return_output(const mx::OutputPtr& output)
        {
            const string& name = output->getName();
            return not serialize::has_prefix(name, serialize::OUT_PARAMETER_PREFIX) and not serialize::has_prefix(name, serialize::NONLOCAL_OUT_PREFIX);
        }

        // single outputs return their type, multiple outputs return a struct, e.g., {float x, float y}
        string return_type(const vector<mx::OutputPtr>& outputs)
        {
            if (outputs.empty())
                return "void";
            if (outputs.size() == 1)
                return type_alias(outputs.front()->getType());

            string fields;
            for (const mx::OutputPtr& output : outputs)
            {
                string field_name = output->getName();
                if (serialize::has_prefix(field_name, serialize::RETURN_VALUE_PREFIX))
                    field_name = serialize::remove_prefix(field_name);
                fields += (fields.empty() ? "" : ", ") + type_alias(output->getType());
                if (not is_index(field_name))
                    fields += " " + to_identifier(field_name);
            }
            return "{" + fields + "}";
        }

        // the variables that functions assign to, i.e., their nonlocal outputs
        unordered_set<string> find_mutable_variables(const mx::DocumentPtr& document)
        {
            unordered_set<string> result;
            for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
            {
                for (const mx::OutputPtr& output : node_def->getActiveOutputs())
                {
                    if (serialize::has_prefix(output->getName(), serialize::NONLOCAL_OUT_PREFIX))
                        result.insert(serialize::remove_prefix(output->getName()));
                }
            }
            return result;
        }

        // e.g., `@uiname "Color" color3 c = color3{1.0}`
        Layout parameter(const vector<string>& attrs, const string& mods, const string& type, const string& name, const optional<Layout>& default_value)
        {
            string declaration;
            for (const string& attr : attrs)
                declaration += attr + " ";
            if (not mods.empty())
                declaration += mods + " ";
            declaration += type + " " + name;
            if (not default_value)
                return declaration;
            return Layout::concat({declaration + " = ", *default_value});
        }

        // the nonlocal variables that the function of the node def reads (nonlocal_in__<name> inputs) or assigns to
        // (nonlocal_out__<name> outputs)
        unordered_set<string> nonlocal_variables(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> result;
            for (const mx::InputPtr& input : node_def->getActiveInputs())
            {
                if (serialize::has_prefix(input->getName(), serialize::NONLOCAL_IN_PREFIX))
                    result.insert(serialize::remove_prefix(input->getName()));
            }
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
            {
                if (serialize::has_prefix(output->getName(), serialize::NONLOCAL_OUT_PREFIX))
                    result.insert(serialize::remove_prefix(output->getName()));
            }
            return result;
        }

        // the nonlocal variables that have no node in the document, e.g., `mutable float x = 0.0;`, which are declared
        // before the first function that uses them or the first value that is assigned to them
        unordered_set<string> find_nonlocal_declarations(const mx::DocumentPtr& document)
        {
            unordered_set<string> result;
            for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
            {
                for (const string& name : nonlocal_variables(node_def))
                {
                    if (document->getNode(name) == nullptr)
                        result.insert(name);
                }
            }
            return result;
        }

        // the variables that are declared outside of the body of the function of a node def, i.e., its parameters (its
        // inputs and the outputs of its out parameters, which are named outparam__<name>) and its nonlocal variables
        unordered_set<string> declared_variables(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> result = nonlocal_variables(node_def);
            for (const mx::InputPtr& input : node_def->getActiveInputs())
            {
                if (not serialize::has_prefix(input->getName(), serialize::NONLOCAL_IN_PREFIX))
                    result.insert(to_identifier(input->getName()));
            }
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
            {
                if (serialize::has_prefix(output->getName(), serialize::OUT_PARAMETER_PREFIX))
                    result.insert(to_identifier(serialize::remove_prefix(output->getName())));
            }
            return result;
        }

        unordered_set<string> declared_variables(const mx::NodeGraphPtr& node_graph)
        {
            unordered_set<string> result;
            for (const mx::InputPtr& input : node_graph->getInputs())
                result.insert(to_identifier(input->getName()));
            return result;
        }

        // e.g., `return x;`, or `return {x, y};` for multiple outputs
        Layout return_statement(vector<Layout> values)
        {
            const Layout value = values.size() == 1 ? values.front() : Layout::list("{", std::move(values), "}");
            return Layout::concat({"return ", value, ";"});
        }
    }

    DocumentDecompiler::DocumentDecompiler(mx::DocumentPtr document)
        : document_{std::move(document)},
        mutable_variables_{find_mutable_variables(document_)},
        graph_decompiler_{*this, document_, find_nonlocal_declarations(document_)}
    {

    }

    string DocumentDecompiler::decompile_document()
    {
        emit_document_attributes();

        for (const mx::ElementPtr& element : document_->getChildren())
        {
            if (const mx::NodeDefPtr node_def = element->asA<mx::NodeDef>())
            {
                emit_function(node_def);
            }
            else if (const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>())
            {
                // node graphs that implement a node def are emitted with their node def
                const mx::NodeDefPtr node_def = node_graph->getNodeDef();
                if (node_def == nullptr or not is_document_node_def(node_def))
                    emit_function(node_graph);
            }
            else if (const mx::NodePtr node = element->asA<mx::Node>())
            {
                if (graph_decompiler_.is_statement(node))
                    emit_node(node);
            }
        }

        return writer_.str();
    }

    string DocumentDecompiler::decompile_node(const mx::NodePtr& node, const bool with_dependencies)
    {
        if (with_dependencies)
        {
            emit_node(node);
            return writer_.str();
        }

        for (Layout& stmt : graph_decompiler_.create_statements(node))
            writer_.add(std::move(stmt));
        return writer_.str();
    }

    string DocumentDecompiler::decompile_function(const mx::ElementPtr& function, const bool with_dependencies)
    {
        emit_function(function);
        if (not with_dependencies)
            writer_.keep_last();
        return writer_.str();
    }

    bool DocumentDecompiler::is_document_node_def(const mx::NodeDefPtr& node_def) const
    {
        return node_def and node_def->getDocument() == document_;
    }

    mx::NodeGraphPtr DocumentDecompiler::implementation(const mx::NodeDefPtr& node_def) const
    {
        for (const mx::ElementPtr& element : document_->getChildren())
        {
            const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>();
            if (node_graph and node_graph->getNodeDefString() == node_def->getName())
                return node_graph;
        }
        return nullptr;
    }

    string DocumentDecompiler::function_name(const mx::ElementPtr& function) const
    {
        if (const mx::NodeDefPtr node_def = function->asA<mx::NodeDef>())
            return to_identifier(node_def->getNodeString());

        // node graphs created by the compiler are named NG_<function name>
        const string& name = function->getName();
        return to_identifier(starts_with(name, "NG_") ? name.substr(3) : name);
    }

    bool DocumentDecompiler::is_required_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        if (not is_document_node_def(node_def))
            return false;
        const mx::InputPtr input = node_def->getActiveInput(input_name);
        return input and has_literal_syntax(input->getType()) and input->hasValue() and is_zero_value(input);
    }

    bool DocumentDecompiler::is_mutable_variable(const string& name) const
    {
        return contains(mutable_variables_, name);
    }

    bool DocumentDecompiler::is_void_function(const mx::NodeDefPtr& node_def) const
    {
        if (not is_document_node_def(node_def))
            return false;

        const vector<mx::OutputPtr> outputs = node_def->getActiveOutputs();
        if (outputs.size() != 1 or outputs.front()->getName() != serialize::RETURN_VALUE_PREFIX or outputs.front()->getType() != "integer")
            return false;

        // `return 0;` is compiled to a constant node, the placeholder is an unconnected output
        const mx::NodeGraphPtr node_graph = implementation(node_def);
        const mx::OutputPtr output = node_graph ? node_graph->getOutput(serialize::RETURN_VALUE_PREFIX) : nullptr;
        return output and output->getNodeName().empty() and output->getInterfaceName().empty() and output->getValueString() == "0";
    }

    void DocumentDecompiler::emit_document_attributes()
    {
        for (const string& attr_name : document_->getAttributeNames())
        {
            if (attr_name == mx::InterfaceElement::VERSION_ATTRIBUTE or attr_name.find(':') != string::npos)
                continue;
            writer_.add("@@" + attr_name + " \"" + document_->getAttribute(attr_name) + "\"");
        }
    }

    void DocumentDecompiler::emit_node(const mx::NodePtr& node)
    {
        if (contains(emitted_nodes_, node))
            return;
        emitted_nodes_.insert(node);

        for (const mx::NodePtr& dependency : graph_decompiler_.statement_dependencies(node))
            emit_node(dependency);

        emit_dependencies(graph_decompiler_.function_dependencies(node));

        // nonlocal variables without a node are declared before the first value that is assigned to them
        if (const optional<string> variable = graph_decompiler_.declared_variable(node))
            emit_nonlocal_variable(nullptr, *variable, node->getType());

        for (Layout& stmt : graph_decompiler_.create_statements(node))
            writer_.add(std::move(stmt));
    }

    void DocumentDecompiler::emit_function(const mx::ElementPtr& function)
    {
        if (contains(emitted_functions_, function))
            return;
        emitted_functions_.insert(function);

        const mx::NodeDefPtr node_def = function->asA<mx::NodeDef>();
        const mx::NodeGraphPtr node_graph = node_def ? implementation(node_def) : function->asA<mx::NodeGraph>();
        if (node_graph == nullptr)
            return;

        // functions are declared after the functions they call and the nonlocal variables they access
        vector<mx::ElementPtr> dependencies;
        const auto add_dependencies = [&](const vector<mx::ElementPtr>& functions) {
            for (const mx::ElementPtr& f : functions)
                if (f != function and f != node_graph and not contains(dependencies, f))
                    dependencies.push_back(f);
        };

        // the variables of the body cannot hide the parameters and nonlocal variables of the function
        GraphDecompiler body{*this, node_graph, node_def ? declared_variables(node_def) : declared_variables(node_graph)};
        for (const mx::NodePtr& node : body.nodes())
            add_dependencies(body.function_dependencies(node));
        for (const mx::OutputPtr& output : node_graph->getOutputs())
            add_dependencies(body.function_dependencies(output));
        emit_dependencies(dependencies);

        // nonlocal variables that the function reads (inputs) or assigns to (outputs)
        vector<mx::ValueElementPtr> ports;
        if (node_def)
        {
            for (const mx::InputPtr& input : node_def->getActiveInputs())
                ports.push_back(input);
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
                ports.push_back(output);
        }
        for (const mx::ValueElementPtr& port : ports)
        {
            const string& name = port->getName();
            if (serialize::has_prefix(name, serialize::NONLOCAL_IN_PREFIX) or serialize::has_prefix(name, serialize::NONLOCAL_OUT_PREFIX))
                emit_nonlocal_variable(node_def, serialize::remove_prefix(name), port->getType());
        }

        // node graph functions can use nodes in the document as default values
        for (const mx::InputPtr& input : node_graph->getInputs())
        {
            if (const mx::NodePtr node = document_->getNode(input->getNodeName()))
                emit_node(node);
        }

        if (node_def)
            writer_.add(create_function_definition(node_def, body), /*is_block*/true);
        else
            writer_.add(create_function_definition(node_graph, body), /*is_block*/true);
    }

    void DocumentDecompiler::emit_dependencies(const vector<mx::ElementPtr>& functions)
    {
        for (const mx::ElementPtr& function : functions)
            emit_function(function);
    }

    void DocumentDecompiler::emit_nonlocal_variable(const mx::NodeDefPtr& node_def, const string& name, const string& type_name)
    {
        if (const mx::NodePtr node = document_->getNode(name); node and graph_decompiler_.is_statement(node))
        {
            emit_node(node);
            return;
        }

        if (contains(emitted_nonlocal_variables_, name))
            return;
        emitted_nonlocal_variables_.insert(name);

        // the value of the variable is passed to each call of the function, use the value of the first call, or of the
        // first call of any function if the variable is declared for a value that is assigned to it
        optional<Code> value;
        for (const mx::NodePtr& node : document_->getNodes())
        {
            const mx::NodeDefPtr call_node_def = node->getNodeDef();
            if (node_def ? call_node_def != node_def : not is_document_node_def(call_node_def))
                continue;
            const mx::InputPtr input = node->getInput(serialize::with_prefix(serialize::NONLOCAL_IN_PREFIX, name));
            if (input and input->getNodeName().empty() and input->hasValue())
            {
                value = literal(input);
                break;
            }
        }

        const bool is_mutable = is_mutable_variable(name) or graph_decompiler_.is_assigned(name);
        string declaration = is_mutable ? "mutable " : "";
        declaration += type_alias(type_name) + " " + name;
        if (value)
            writer_.add(Layout::concat({declaration + " = ", value->layout, ";"}));
        else
            writer_.add(declaration + ";");
    }

    Layout DocumentDecompiler::create_function_definition(const mx::NodeDefPtr& node_def, GraphDecompiler& body)
    {
        const mx::NodeGraphPtr node_graph = implementation(node_def);

        vector<mx::OutputPtr> return_outputs;
        vector<mx::OutputPtr> out_parameter_outputs;
        vector<string> attrs = user_attributes(node_def);
        const bool is_void = is_void_function(node_def);
        for (const mx::OutputPtr& output : node_def->getActiveOutputs())
        {
            if (is_return_output(output) and not is_void)
            {
                return_outputs.push_back(output);
                const vector<string> output_attrs = user_attributes(output, output->getName());
                attrs.insert(attrs.end(), output_attrs.begin(), output_attrs.end());
            }
            else if (serialize::has_prefix(output->getName(), serialize::OUT_PARAMETER_PREFIX))
            {
                out_parameter_outputs.push_back(output);
            }
        }

        // parameters without a default value are declared with the default of their type, e.g., 0.0
        vector<Layout> params;
        for (const mx::InputPtr& input : node_def->getActiveInputs())
        {
            if (serialize::has_prefix(input->getName(), serialize::NONLOCAL_IN_PREFIX))
                continue;

            // ref parameters are both an input and an out parameter output
            const bool is_ref = node_def->getActiveOutput(serialize::with_prefix(serialize::OUT_PARAMETER_PREFIX, input->getName())) != nullptr;

            optional<Layout> default_value;
            if (is_ref)
                default_value = std::nullopt;
            else if (not has_literal_syntax(input->getType()) or not input->hasValue())
                default_value = "null";
            else if (not is_zero_value(input))
                default_value = literal(input)->layout;

            params.push_back(parameter(user_attributes(input), is_ref ? "ref" : "", type_alias(input->getType()), to_identifier(input->getName()), default_value));
        }

        for (const mx::OutputPtr& output : out_parameter_outputs)
        {
            if (node_def->getActiveInput(serialize::remove_prefix(output->getName())))
                continue;

            const string name = to_identifier(serialize::remove_prefix(output->getName()));
            params.push_back(parameter(user_attributes(output), "out", type_alias(output->getType()), name, std::nullopt));
        }

        vector<Layout> body_statements = create_body_statements(body);

        for (const mx::OutputPtr& output : node_graph->getOutputs())
        {
            const string& name = output->getName();
            const bool is_out_parameter = serialize::has_prefix(name, serialize::OUT_PARAMETER_PREFIX);
            const bool is_nonlocal = serialize::has_prefix(name, serialize::NONLOCAL_OUT_PREFIX);
            if (not is_out_parameter and not is_nonlocal)
                continue;

            const optional<Code> value = body.create_port_expression(output);
            if (not value)
                continue;

            // the value is already assigned to the variable, e.g., `total += x;`
            const string var_name = is_out_parameter ? to_identifier(serialize::remove_prefix(name)) : serialize::remove_prefix(name);
            if (body.has_assigned_value(output, var_name))
                continue;

            body_statements.push_back(Layout::concat({var_name + " = ", value->layout, ";"}));
        }

        if (not return_outputs.empty())
        {
            vector<Layout> return_values;
            for (const mx::OutputPtr& output : return_outputs)
                return_values.push_back(output_value(body, node_graph->getOutput(output->getName())));
            body_statements.push_back(return_statement(std::move(return_values)));
        }

        // the modifier is written above the function, after its attributes
        const Layout header = Layout::concat({return_type(return_outputs) + " " + function_name(node_def), Layout::parameter_list(std::move(params))});
        return code::with_attributes(attrs, Layout::block(Layout::lines({"[[nodedef]]", header}), std::move(body_statements)));
    }

    Layout DocumentDecompiler::create_function_definition(const mx::NodeGraphPtr& node_graph, GraphDecompiler& body)
    {
        // node graph functions have default values for all of their parameters and are called without arguments
        vector<Layout> params;
        for (const mx::InputPtr& input : node_graph->getInputs())
        {
            const optional<Code> default_value = graph_decompiler_.create_port_expression(input);
            const Layout value = default_value ? default_value->layout : "null";
            params.push_back(parameter(user_attributes(input), "", type_alias(input->getType()), to_identifier(input->getName()), value));
        }

        vector<Layout> body_statements = create_body_statements(body);

        const vector<mx::OutputPtr> outputs = node_graph->getOutputs();
        if (not outputs.empty())
        {
            vector<Layout> return_values;
            for (const mx::OutputPtr& output : outputs)
                return_values.push_back(output_value(body, output));
            body_statements.push_back(return_statement(std::move(return_values)));
        }

        // parameterless functions are node graphs by default, e.g., `float f => { ... }`, and the modifier of other node
        // graph functions is written above them, after their attributes
        const string declaration = return_type(outputs) + " " + function_name(node_graph);
        const Layout header = params.empty()
            ? declaration + " =>"
            : Layout::lines({"[[nodegraph]]", Layout::concat({declaration, Layout::parameter_list(std::move(params))})});
        return code::with_attributes(user_attributes(node_graph), Layout::block(header, std::move(body_statements)));
    }

    vector<Layout> DocumentDecompiler::create_body_statements(GraphDecompiler& body)
    {
        vector<Layout> result;
        for (const mx::NodePtr& node : body.ordered_statements())
        {
            for (Layout& stmt : body.create_statements(node))
                result.push_back(std::move(stmt));
        }
        return result;
    }

    Layout DocumentDecompiler::output_value(GraphDecompiler& body, const mx::OutputPtr& output)
    {
        const optional<Code> value = output ? body.create_port_expression(output) : std::nullopt;
        return value ? value->layout : "null";
    }
}
