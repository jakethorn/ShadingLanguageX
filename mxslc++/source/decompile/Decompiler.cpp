//
// Created by jaket on 28/09/2026.
//

#include "decompile/Decompiler.h"

#include <MaterialXFormat/XmlIo.h>

#include "decompile/decompile_utils.h"
#include "errors/CompileError.h"
#include "utils/io_utils.h"
#include "utils/load_mtlx.h"
#include "serialize/name_prefix_utils.h"
#include "utils/container_utils.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"

namespace mxslc::decompile
{
    using namespace decompile_utils;
    using container_utils::contains;
    using string_utils::starts_with;

    namespace
    {
        mx::DocumentPtr read_document(const fs::path& src_path)
        {
            const mx::DocumentPtr document = mx::createDocument();
            mx::readFromXmlFile(document, src_path.string());
            return document;
        }

        mx::DocumentPtr read_document(const string& source)
        {
            const mx::DocumentPtr document = mx::createDocument();
            mx::readFromXmlString(document, source);
            return document;
        }

        // the node defs of the MaterialX libraries are needed to know the order and default values of node inputs
        // a copy of the document with the MaterialX libraries of its version, whose elements are sorted into the order of
        // code, see sort_by_dependencies
        mx::DocumentPtr copy_document(const mx::DocumentPtr& document)
        {
            const mx::DocumentPtr copy = mx::createDocument();
            copy->copyContentFrom(document);
            copy->setDataLibrary(load_materialx_library(copy->getVersionString(), io_utils::get_default_search_directories()));
            sort_by_dependencies(copy);
            return copy;
        }

        // the elements that are passed to the decompiler can belong to the original document instead of its copy
        template <typename T>
        shared_ptr<T> find_copy(const mx::DocumentPtr& document, const shared_ptr<T>& element)
        {
            if (element == nullptr)
                throw CompileError{"Cannot find element"};
            if (element->getDocument() == document)
                return element;
            if (const mx::ElementPtr& copy = document->getDescendant(element->getNamePath()))
                return copy->asA<T>();
            throw CompileError{"Cannot find element at path: " + element->getNamePath()};
        }

        unordered_set<string> find_nonlocal_inputs(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> nonlocal_inputs;
            for (const mx::InputPtr& input : node_def->getActiveInputs())
            {
                if (has_prefix(input, NONLOCAL_IN_PREFIX))
                    nonlocal_inputs.insert(without_prefix(input));
            }
            return nonlocal_inputs;
        }

        unordered_set<string> find_nonlocal_inputs(const mx::DocumentPtr& document)
        {
            unordered_set<string> nonlocal_inputs;
            for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
                nonlocal_inputs.merge(find_nonlocal_inputs(node_def));
            return nonlocal_inputs;
        }

        unordered_set<string> find_nonlocal_outputs(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> nonlocal_outputs;
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
            {
                if (has_prefix(output, NONLOCAL_OUT_PREFIX))
                    nonlocal_outputs.insert(without_prefix(output));
            }
            return nonlocal_outputs;
        }

        unordered_set<string> find_nonlocal_outputs(const mx::DocumentPtr& document)
        {
            unordered_set<string> nonlocal_outputs;
            for (const mx::NodeDefPtr& node_def : document->getNodeDefs())
                nonlocal_outputs.merge(find_nonlocal_outputs(node_def));
            return nonlocal_outputs;
        }

        unordered_set<string> find_nonlocals(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> nonlocals = find_nonlocal_inputs(node_def);
            nonlocals.merge(find_nonlocal_outputs(node_def));
            return nonlocals;
        }

        unordered_set<string> find_nonlocals(const mx::DocumentPtr& document)
        {
            unordered_set<string> nonlocals = find_nonlocal_inputs(document);
            nonlocals.merge(find_nonlocal_outputs(document));
            return nonlocals;
        }

        // find the nonlocal variables that have no node in the document, e.g., `mutable float x = 0.0;`
        unordered_set<string> find_nonlocal_declarations(const mx::DocumentPtr& document)
        {
            unordered_set<string> result;
            for (const string& name : find_nonlocals(document))
            {
                if (document->getNode(name) == nullptr)
                    result.insert(name);
            }
            return result;
        }

        unordered_set<string> find_parameters(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> parameters;
            for (const mx::InputPtr& input : node_def->getActiveInputs())
            {
                if (not has_prefix(input, NONLOCAL_IN_PREFIX))
                    parameters.insert(input->getName());
            }
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
            {
                if (has_prefix(output, OUT_PARAMETER_PREFIX))
                    parameters.insert(without_prefix(output));
            }
            return parameters;
        }

        // find the variables that are declared outside of the body of the function,
        // i.e., its parameters and nonlocal variables
        unordered_set<string> find_outside_variables(const mx::NodeDefPtr& node_def)
        {
            unordered_set<string> result = find_nonlocals(node_def);
            result.merge(find_parameters(node_def));
            return result;
        }

        unordered_set<string> find_outside_variables(const mx::NodeGraphPtr& node_graph)
        {
            unordered_set<string> result;
            for (const mx::InputPtr& input : node_graph->getInputs())
                result.insert(input->getName());
            return result;
        }

        // e.g., `@uiname "Color" color3 c = color3{1.0}`
        SourceCode format_parameter(const vector<string>& attrs, const string& mods, const string& type, const string& name, const optional<SourceCode>& default_value)
        {
            string declaration;
            for (const string& attr : attrs)
                declaration += attr + " ";
            if (not mods.empty())
                declaration += mods + " ";
            declaration += type + " " + name;
            if (not default_value)
                return declaration;
            return SourceCode::concat({declaration + " = ", *default_value});
        }

        // e.g., `return x;`, or `return {x, y};` for multiple outputs
        SourceCode format_return_statement(vector<SourceCode> values)
        {
            const SourceCode value = values.size() == 1 ? values.front() : SourceCode::list("{", std::move(values), "}");
            return SourceCode::concat({"return ", value, ";"});
        }
    }

    Decompiler::Decompiler(const fs::path& src_path) : Decompiler{read_document(src_path)}
    {

    }

    Decompiler::Decompiler(const string& source) : Decompiler{read_document(source)}
    {

    }

    Decompiler::Decompiler(const mx::DocumentPtr& document)
        : document_{copy_document(document)},
        function_assigned_variables_{find_nonlocal_outputs(document_)},
        graph_decompiler_{*this, document_, find_nonlocal_declarations(document_)}
    {

    }

    string Decompiler::decompile_document()
    {
        clear_emitted_code();
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
                if (node_graph->getNodeDef() == nullptr)
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

    string Decompiler::decompile_node(const string& node_name, const bool with_dependencies)
    {
        return decompile_node(document_->getNode(node_name), with_dependencies);
    }

    string Decompiler::decompile_node(const mx::NodePtr& node, const bool with_dependencies)
    {
        const mx::NodePtr copy = find_copy<mx::Node>(document_, node);
        clear_emitted_code();

        if (with_dependencies)
        {
            emit_node(copy);
            return writer_.str();
        }

        for (SourceCode& stmt : graph_decompiler_.create_statements(copy))
            writer_.add(std::move(stmt));
        return writer_.str();
    }

    string Decompiler::decompile_node_def(const string& node_def_name, const bool with_dependencies)
    {
        return decompile_node_def(document_->getNodeDef(node_def_name), with_dependencies);
    }

    string Decompiler::decompile_node_def(const mx::NodeDefPtr& node_def, const bool with_dependencies)
    {
        return decompile_function(find_copy<mx::NodeDef>(document_, node_def), with_dependencies);
    }

    string Decompiler::decompile_node_graph(const string& node_graph_name, const bool with_dependencies)
    {
        return decompile_node_graph(document_->getNodeGraph(node_graph_name), with_dependencies);
    }

    string Decompiler::decompile_node_graph(const mx::NodeGraphPtr& node_graph, const bool with_dependencies)
    {
        const mx::NodeGraphPtr copy = find_copy<mx::NodeGraph>(document_, node_graph);

        // node graphs that implement a node def are decompiled as part of the node def
        if (const mx::NodeDefPtr node_def = copy->getNodeDef(); is_document_node_def(node_def))
            return decompile_function(node_def, with_dependencies);

        return decompile_function(copy, with_dependencies);
    }

    string Decompiler::decompile_function(const mx::ElementPtr& function, const bool with_dependencies)
    {
        clear_emitted_code();
        emit_function(function);
        if (not with_dependencies)
            writer_.keep_last();
        return writer_.str();
    }

    void Decompiler::clear_emitted_code()
    {
        writer_.clear();
        emitted_nodes_.clear();
        emitted_functions_.clear();
        emitted_nonlocal_variables_.clear();
    }

    bool Decompiler::is_document_node_def(const mx::NodeDefPtr& node_def) const
    {
        return node_def and node_def->getDocument() == document_;
    }

    string Decompiler::get_function_name(const mx::ElementPtr& function) const
    {
        if (const mx::NodeDefPtr node_def = function->asA<mx::NodeDef>())
            return make_identifier(node_def->getNodeString());

        // node graphs created by the compiler are named NG_<function name>
        const string& name = function->getName();
        return make_identifier(starts_with(name, "NG_") ? name.substr(3) : name);
    }

    bool Decompiler::is_required_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        if (not is_document_node_def(node_def))
            return false;
        const mx::InputPtr input = node_def->getActiveInput(input_name);
        return input and has_literal_syntax(input->getType()) and input->hasValue() and is_zero_value(input);
    }

    bool Decompiler::is_assigned_by_function(const string& variable) const
    {
        return contains(function_assigned_variables_, variable);
    }

    bool Decompiler::is_void_function(const mx::NodeDefPtr& node_def) const
    {
        if (not is_document_node_def(node_def))
            return false;

        const vector<mx::OutputPtr> outputs = node_def->getActiveOutputs();
        if (outputs.size() != 1 or outputs.front()->getName() != serialize::RETURN_VALUE_PREFIX or outputs.front()->getType() != "integer")
            return false;

        // `return 0;` is compiled to a constant node, the placeholder is an unconnected output
        const mx::NodeGraphPtr node_graph = mtlx_utils::get_node_graph(node_def);
        const mx::OutputPtr output = node_graph ? node_graph->getOutput(serialize::RETURN_VALUE_PREFIX) : nullptr;
        return output and output->getNodeName().empty() and output->getInterfaceName().empty() and output->getValueString() == "0";
    }

    void Decompiler::emit_document_attributes()
    {
        for (const string& attr_name : document_->getAttributeNames())
        {
            if (attr_name == mx::InterfaceElement::VERSION_ATTRIBUTE)
                continue;
            writer_.add("@@" + attr_name + " \"" + document_->getAttribute(attr_name) + "\"");
        }
    }

    void Decompiler::emit_node(const mx::NodePtr& node)
    {
        if (contains(emitted_nodes_, node))
            return;
        emitted_nodes_.insert(node);

        for (const mx::NodePtr& dependency : graph_decompiler_.get_statement_dependencies(node))
            emit_node(dependency);

        emit_dependencies(graph_decompiler_.get_function_dependencies(node));

        // nonlocal variables without a node are declared before the first value that is assigned to them
        if (const optional<string> variable = graph_decompiler_.get_outside_variable(node))
            emit_nonlocal_variable(nullptr, *variable, node->getType());

        for (SourceCode& stmt : graph_decompiler_.create_statements(node))
            writer_.add(std::move(stmt));
    }

    void Decompiler::emit_function(const mx::ElementPtr& function)
    {
        if (contains(emitted_functions_, function))
            return;
        emitted_functions_.insert(function);

        const mx::NodeDefPtr node_def = function->asA<mx::NodeDef>();
        const mx::NodeGraphPtr node_graph = node_def ? mtlx_utils::get_node_graph(node_def) : function->asA<mx::NodeGraph>();
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
        GraphDecompiler body{*this, node_graph, node_def ? find_outside_variables(node_def) : find_outside_variables(node_graph)};
        for (const mx::NodePtr& node : body.nodes())
            add_dependencies(body.get_function_dependencies(node));
        for (const mx::OutputPtr& output : node_graph->getOutputs())
            add_dependencies(body.get_function_dependencies(output));
        emit_dependencies(dependencies);

        // nonlocal variables that the function reads (inputs) or assigns to (outputs)
        vector<mx::PortElementPtr> ports;
        if (node_def)
        {
            for (const mx::InputPtr& input : node_def->getActiveInputs())
                ports.push_back(input);
            for (const mx::OutputPtr& output : node_def->getActiveOutputs())
                ports.push_back(output);
        }
        for (const mx::PortElementPtr& port : ports)
        {
            if (has_prefix(port, NONLOCAL_IN_PREFIX) or has_prefix(port, NONLOCAL_OUT_PREFIX))
                emit_nonlocal_variable(node_def, without_prefix(port), port->getType());
        }

        // node graph functions can use nodes and the outputs of node graphs in the document as default values, e.g.,
        // `float foo(float b = bar())`
        for (const mx::InputPtr& input : node_graph->getInputs())
        {
            if (const mx::NodePtr node = document_->getNode(input->getNodeName()))
                emit_node(node);
            if (const mx::NodeGraphPtr default_graph = document_->getNodeGraph(input->getNodeGraphString()))
                emit_function(default_graph);
        }

        if (node_def)
            writer_.add(create_function_definition(node_def, body), /*is_block*/true);
        else
            writer_.add(create_function_definition(node_graph, body), /*is_block*/true);
    }

    void Decompiler::emit_dependencies(const vector<mx::ElementPtr>& functions)
    {
        for (const mx::ElementPtr& function : functions)
            emit_function(function);
    }

    void Decompiler::emit_nonlocal_variable(const mx::NodeDefPtr& node_def, const string& name, const string& type_name)
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
        optional<ExpressionCode> value;
        for (const mx::NodePtr& node : document_->getNodes())
        {
            const mx::NodeDefPtr call_node_def = node->getNodeDef();
            if (node_def ? call_node_def != node_def : not is_document_node_def(call_node_def))
                continue;
            const mx::InputPtr input = node->getInput(serialize::with_prefix(serialize::NONLOCAL_IN_PREFIX, name));
            if (input and input->getNodeName().empty() and input->hasValue())
            {
                value = format_value(input);
                break;
            }
        }

        const bool is_mutable = is_assigned_by_function(name) or graph_decompiler_.is_assigned(name);
        string declaration = is_mutable ? "mutable " : "";
        declaration += get_type_alias(type_name) + " " + name;
        if (value)
            writer_.add(SourceCode::concat({declaration + " = ", value->code, ";"}));
        else
            writer_.add(declaration + ";");
    }

    SourceCode Decompiler::create_function_definition(const mx::NodeDefPtr& node_def, GraphDecompiler& body)
    {
        const mx::NodeGraphPtr node_graph = mtlx_utils::get_node_graph(node_def);

        vector<mx::OutputPtr> return_outputs;
        vector<mx::OutputPtr> out_parameter_outputs;
        vector<string> attrs = get_user_attributes(node_def);
        const bool is_void = is_void_function(node_def);
        for (const mx::OutputPtr& output : node_def->getActiveOutputs())
        {
            if (is_return_output(output) and not is_void)
            {
                return_outputs.push_back(output);
                const vector<string> output_attrs = get_user_attributes(output, output->getName());
                attrs.insert(attrs.end(), output_attrs.begin(), output_attrs.end());
            }
            else if (has_prefix(output, OUT_PARAMETER_PREFIX))
            {
                out_parameter_outputs.push_back(output);
            }
        }

        // parameters without a default value are declared with the default of their type, e.g., 0.0
        vector<SourceCode> params;
        for (const mx::InputPtr& input : node_def->getActiveInputs())
        {
            if (has_prefix(input, NONLOCAL_IN_PREFIX))
                continue;

            // ref parameters are both an input and an out parameter output
            const bool is_ref = node_def->getActiveOutput(with_prefix(OUT_PARAMETER_PREFIX, input->getName())) != nullptr;

            optional<SourceCode> default_value;
            if (is_ref)
                default_value = std::nullopt;
            else if (not has_literal_syntax(input->getType()) or not input->hasValue())
                default_value = "null";
            else if (not is_zero_value(input))
                default_value = format_value(input)->code;

            params.push_back(format_parameter(get_user_attributes(input), is_ref ? "ref" : "", get_type_alias(input->getType()), make_identifier(input->getName()), default_value));
        }

        for (const mx::OutputPtr& output : out_parameter_outputs)
        {
            if (node_def->getActiveInput(serialize::remove_prefix(output->getName())))
                continue;

            const string name = make_identifier(serialize::remove_prefix(output->getName()));
            params.push_back(format_parameter(get_user_attributes(output), "out", get_type_alias(output->getType()), name, std::nullopt));
        }

        vector<SourceCode> body_statements = create_body_statements(body);

        for (const mx::OutputPtr& output : node_graph->getOutputs())
        {
            const string& name = output->getName();
            const bool is_out_parameter = has_prefix(name, OUT_PARAMETER_PREFIX);
            const bool is_nonlocal = has_prefix(name, NONLOCAL_OUT_PREFIX);
            if (not is_out_parameter and not is_nonlocal)
                continue;

            const optional<ExpressionCode> value = body.create_port_expression(output);
            if (not value)
                continue;

            // the value is already assigned to the variable, e.g., `total += x;`
            const string var_name = is_out_parameter ? make_identifier(remove_prefix(name)) : remove_prefix(name);
            if (body.has_assigned_value(output, var_name))
                continue;

            body_statements.push_back(SourceCode::concat({var_name + " = ", value->code, ";"}));
        }

        if (not return_outputs.empty())
        {
            vector<SourceCode> return_values;
            for (const mx::OutputPtr& output : return_outputs)
                return_values.push_back(create_output_value(body, node_graph->getOutput(output->getName())));
            body_statements.push_back(format_return_statement(std::move(return_values)));
        }

        // the modifier is written above the function, after its attributes
        const SourceCode header = SourceCode::concat({get_return_type(return_outputs) + " " + get_function_name(node_def), SourceCode::parameter_list(std::move(params))});
        return add_attributes(attrs, SourceCode::block(SourceCode::lines({"[[nodedef]]", header}), std::move(body_statements)));
    }

    SourceCode Decompiler::create_function_definition(const mx::NodeGraphPtr& node_graph, GraphDecompiler& body)
    {
        // node graph functions have default values for all of their parameters and are called without arguments
        vector<SourceCode> params;
        for (const mx::InputPtr& input : node_graph->getInputs())
        {
            const optional<ExpressionCode> default_value = graph_decompiler_.create_port_expression(input);
            const SourceCode value = default_value ? default_value->code : "null";
            params.push_back(format_parameter(get_user_attributes(input), "", get_type_alias(input->getType()), make_identifier(input->getName()), value));
        }

        vector<SourceCode> body_statements = create_body_statements(body);

        const vector<mx::OutputPtr> outputs = node_graph->getOutputs();
        if (not outputs.empty())
        {
            vector<SourceCode> return_values;
            for (const mx::OutputPtr& output : outputs)
                return_values.push_back(create_output_value(body, output));
            body_statements.push_back(format_return_statement(std::move(return_values)));
        }

        // parameterless functions are node graphs by default, e.g., `float f => { ... }`, and the modifier of other node
        // graph functions is written above them, after their attributes
        const string declaration = get_return_type(outputs) + " " + get_function_name(node_graph);
        const SourceCode header = params.empty()
            ? declaration + " =>"
            : SourceCode::lines({"[[nodegraph]]", SourceCode::concat({declaration, SourceCode::parameter_list(std::move(params))})});
        return add_attributes(get_user_attributes(node_graph), SourceCode::block(header, std::move(body_statements)));
    }

    vector<SourceCode> Decompiler::create_body_statements(GraphDecompiler& body)
    {
        vector<SourceCode> result;
        for (const mx::NodePtr& node : body.get_ordered_statements())
        {
            for (SourceCode& stmt : body.create_statements(node))
                result.push_back(std::move(stmt));
        }
        return result;
    }

    SourceCode Decompiler::create_output_value(GraphDecompiler& body, const mx::OutputPtr& output)
    {
        const optional<ExpressionCode> value = output ? body.create_port_expression(output) : std::nullopt;
        return value ? value->code : "null";
    }
}
