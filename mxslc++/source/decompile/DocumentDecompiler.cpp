//
// Created by jaket on 28/09/2026.
//

#include "decompile/DocumentDecompiler.h"

#include <functional>

#include "decompile/decompile_utils.h"
#include "expressions/NullExpression.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/interface.h"
#include "parse.h"
#include "runtime/Attribute.h"
#include "runtime/Field.h"
#include "runtime/Parameter.h"
#include "runtime/ParameterList.h"
#include "runtime/Type.h"
#include "runtime/interface.h"
#include "serialize/serialize_name_utils.h"
#include "statements/BlockStatement.h"
#include "statements/DocumentAttribute.h"
#include "statements/FunctionDefinition.h"
#include "statements/ReturnStatement.h"
#include "statements/VariableDefinition.h"
#include "statements/VariableAssignment.h"
#include "statements/interface.h"
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

        bool is_implicit_input(const mx::InputPtr& input)
        {
            const string& name = input->getName();
            return serialize::has_prefix(name, serialize::NONLOCAL_IN_PREFIX);
        }

        // single outputs return their type, multiple outputs return a struct, e.g., {float x, float y}
        TypePtr return_type(const vector<mx::OutputPtr>& outputs)
        {
            if (outputs.empty())
                return create_type("void");
            if (outputs.size() == 1)
                return create_type_from(outputs.front()->getType());

            vector<Field> fields;
            for (const mx::OutputPtr& output : outputs)
            {
                string field_name = output->getName();
                if (serialize::has_prefix(field_name, serialize::RETURN_VALUE_PREFIX))
                    field_name = serialize::remove_prefix(field_name);
                const bool is_index = std::all_of(field_name.begin(), field_name.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
                fields.emplace_back(create_type_from(output->getType()), is_index ? "" : to_identifier(field_name));
            }
            return create_type(std::move(fields));
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

        AttributeList concat(const AttributeList& a, const AttributeList& b)
        {
            vector<Attribute> attrs = a.attributes();
            attrs.insert(attrs.end(), b.attributes().begin(), b.attributes().end());
            return AttributeList{std::move(attrs)};
        }
    }

    DocumentDecompiler::DocumentDecompiler(mx::DocumentPtr document, const HintUsage hint_usage)
        : document_{std::move(document)},
        hint_usage_{hint_usage},
        hints_{hint_usage != HintUsage::None ? DocumentHints{document_} : DocumentHints{}},
        mutable_variables_{find_mutable_variables(document_)},
        graph_decompiler_{*this, document_}
    {

    }

    string DocumentDecompiler::decompile_document()
    {
        emit_document_attributes();

        // statements with hints are emitted in the order of the code, the rest in the order of the document
        for (const size_t id : hints_.children(0))
            emit_hinted_statement(id);

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

        return print();
    }

    string DocumentDecompiler::decompile_node(const mx::NodePtr& node, const bool with_dependencies)
    {
        if (with_dependencies)
        {
            emit_node(node);
            return print();
        }

        graph_decompiler_.create_statements(node, statements_);
        return print();
    }

    string DocumentDecompiler::decompile_function(const mx::ElementPtr& function, const bool with_dependencies)
    {
        emit_function(function);

        if (not with_dependencies and not statements_.empty())
        {
            StmtPtr function_definition = std::move(statements_.back());
            statements_.clear();
            statements_.push_back(std::move(function_definition));
        }

        return print();
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
        if (const auto it = template_instances_.find(function->getName()); it != template_instances_.end())
            return template_definition(it->second)->name();

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

    bool DocumentDecompiler::is_hinted_variable(const string& name) const
    {
        const vector<string> path = split_string(name, "__");
        return graph_decompiler_.is_hinted_variable(name) or (path.size() > 1 and graph_decompiler_.is_hinted_variable(path.front()));
    }

    bool DocumentDecompiler::is_inline_function_defined(const size_t definition_id) const
    {
        return contains(inline_functions_, definition_id);
    }

    vector<mx::NodeDefPtr> DocumentDecompiler::template_instances(const mx::NodeDefPtr& node_def) const
    {
        const auto it = template_instances_.find(node_def->getName());
        if (it == template_instances_.end())
            return {};

        vector<mx::NodeDefPtr> result;
        for (const mx::ElementPtr& instance : template_functions_.at(it->second).instances)
        {
            if (const mx::NodeDefPtr instance_node_def = instance->asA<mx::NodeDef>())
                result.push_back(instance_node_def);
        }
        return result;
    }

    TypePtr DocumentDecompiler::template_type(const mx::NodeDefPtr& node_def) const
    {
        const auto it = template_instances_.find(node_def->getName());
        if (it == template_instances_.end())
            return nullptr;

        // the node defs are created in the order of the template types
        const vector<mx::ElementPtr>& instances = template_functions_.at(it->second).instances;
        const size_t index = std::find(instances.begin(), instances.end(), node_def) - instances.begin();
        const vector<TypePtr>& types = template_definition(it->second)->template_types();
        return index < types.size() ? types[index] : nullptr;
    }

    vector<mx::ElementPtr> DocumentDecompiler::function_elements(const size_t definition_id) const
    {
        // node graphs that implement a node def are part of the function of the node def
        vector<mx::ElementPtr> result;
        for (const mx::ElementPtr& element : document_->getChildren())
        {
            const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>();
            const bool is_function = element->isA<mx::NodeDef>() or (node_graph and not is_document_node_def(node_graph->getNodeDef()));
            if (is_function and DocumentHints::definition(element) == definition_id)
                result.push_back(element);
        }
        return result;
    }

    shared_ptr<FunctionDefinition> DocumentDecompiler::template_definition(const size_t definition_id) const
    {
        const DocumentHints::StatementRecord* record = hints_.statement(definition_id);
        const auto func_def = record ? std::dynamic_pointer_cast<FunctionDefinition>(record->statement) : nullptr;
        if (func_def == nullptr or func_def->is_inline() or func_def->template_types().empty())
            return nullptr;
        return func_def;
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
            if (attr_name == mx::InterfaceElement::VERSION_ATTRIBUTE or attr_name.find(':') != string::npos or DocumentHints::is_hint(attr_name))
                continue;
            Attribute attr{attr_name, document_->getAttribute(attr_name)};
            statements_.push_back(statements::create_statement<DocumentAttribute>(Token{}, std::move(attr)));
        }
    }

    void DocumentDecompiler::emit_hinted_statement(const size_t id)
    {
        const DocumentHints::StatementRecord* record = hints_.statement(id);
        if (record == nullptr or record->statement == nullptr)
            return;

        if (const auto func_def = std::dynamic_pointer_cast<FunctionDefinition>(record->statement))
        {
            // inline functions are recreated from their calls, other functions from their node def or node graph, and
            // comptime functions are not decompiled
            if (func_def->is_inline())
            {
                if (not hints_.is_inline_function(id))
                    return;
                if (StmtPtr definition = graph_decompiler_.create_inline_function(id))
                {
                    statements_.push_back(std::move(definition));
                    inline_functions_.insert(id);
                }
            }
            else
            {
                for (const mx::ElementPtr& function : function_elements(id))
                    emit_function(function);
            }
            return;
        }

        if (not graph_decompiler_.has_hinted_statement(id))
            return;

        emit_dependencies(graph_decompiler_.hinted_function_dependencies(id));
        for (const mx::NodePtr& node : graph_decompiler_.hinted_statement_dependencies(id))
            emit_node(node);
        graph_decompiler_.create_hinted_statements(id, statements_);
    }

    void DocumentDecompiler::emit_node(const mx::NodePtr& node)
    {
        // nodes with hints are emitted by the statements that created them
        if (contains(emitted_nodes_, node) or graph_decompiler_.is_hinted(node))
            return;
        emitted_nodes_.insert(node);

        for (const mx::NodePtr& dependency : graph_decompiler_.statement_dependencies(node))
            emit_node(dependency);

        emit_dependencies(graph_decompiler_.function_dependencies(node));

        graph_decompiler_.create_statements(node, statements_);
    }

    void DocumentDecompiler::emit_function(const mx::ElementPtr& function)
    {
        if (contains(emitted_functions_, function))
            return;

        // the node defs or node graphs of a templated function are emitted together, as the templated function
        const optional<size_t> definition_id = DocumentHints::definition(function);
        if (definition_id and template_definition(*definition_id) and emit_template_function(*definition_id))
            return;

        emitted_functions_.insert(function);

        const mx::NodeDefPtr node_def = function->asA<mx::NodeDef>();
        const mx::NodeGraphPtr node_graph = node_def ? implementation(node_def) : function->asA<mx::NodeGraph>();
        if (node_graph == nullptr)
            return;

        emit_function_dependencies(function);

        if (node_def)
            statements_.push_back(create_function_definition(node_def));
        else
            statements_.push_back(create_function_definition(node_graph));
    }

    bool DocumentDecompiler::emit_template_function(const size_t definition_id)
    {
        if (const auto it = template_functions_.find(definition_id); it != template_functions_.end())
            return it->second.state == TemplateState::Merged;

        TemplateFunction& function = template_functions_[definition_id];
        const shared_ptr<FunctionDefinition> func_def = template_definition(definition_id);
        function.instances = function_elements(definition_id);

        // e.g., a node def was removed
        if (func_def == nullptr or function.instances.size() != func_def->template_types().size())
        {
            function.state = TemplateState::Separate;
            return false;
        }

        for (const mx::ElementPtr& instance : function.instances)
            emit_function_dependencies(instance);

        // the bodies are the same if they only depend on the template type through T, which is guessed from the names of
        // the template types in their code, e.g., `vec2{1.0}` is `T{1.0}` for the function of vec2
        vector<Token> body;
        AttributeList attrs;
        for (size_t i = 0; i < function.instances.size(); ++i)
        {
            const mx::ElementPtr& instance = function.instances[i];
            const mx::NodeDefPtr node_def = instance->asA<mx::NodeDef>();
            const StmtPtr definition = node_def ? create_function_definition(node_def) : create_function_definition(instance->asA<mx::NodeGraph>());
            const auto* instance_def = dynamic_cast<const FunctionDefinition*>(definition.get());
            if (instance_def == nullptr or instance_def->body() == nullptr)
            {
                function.state = TemplateState::Separate;
                return false;
            }

            vector<Token> instance_body = DocumentHints::scan_skeleton(instance_def->body()->to_string());
            const unordered_set<string> type_names = template_type_names(definition_id, i);
            for (Token& token : instance_body)
            {
                if (contains(type_names, token.lexeme()))
                    token = Token{TokenType::Identifier, "T"};
            }

            if (i == 0)
            {
                body = std::move(instance_body);
                attrs = instance_def->attributes();
            }
            else if (instance_body != body or instance_def->attributes().to_string() != attrs.to_string())
            {
                function.state = TemplateState::Separate;
                return false;
            }
        }

        StmtPtr body_stmt;
        try
        {
            Parser parser{std::move(body)};
            body_stmt = parser.block_statement();
        }
        catch (...)
        {
        }
        if (body_stmt == nullptr)
        {
            function.state = TemplateState::Separate;
            return false;
        }

        StmtPtr templated = func_def->with_body(std::move(body_stmt));
        templated->set_attributes(std::move(attrs));
        statements_.push_back(std::move(templated));

        function.state = TemplateState::Merged;
        for (const mx::ElementPtr& instance : function.instances)
        {
            emitted_functions_.insert(instance);
            template_instances_[instance->getName()] = definition_id;
        }
        return true;
    }

    unordered_set<string> DocumentDecompiler::template_type_names(const size_t definition_id, const size_t index) const
    {
        const shared_ptr<FunctionDefinition> func_def = template_definition(definition_id);
        const vector<mx::ElementPtr>& instances = template_functions_.at(definition_id).instances;

        // the code recorded by the hints is the code of the first template type, e.g., `vector2{1.0}`
        unordered_set<string> result;
        for (const size_t i : {size_t{0}, index})
        {
            result.insert(func_def->template_types()[i]->to_string());

            // the node defs and node graphs are named after the function and their type, e.g., foo_vector2
            const mx::NodeDefPtr node_def = instances[i]->asA<mx::NodeDef>();
            const string& name = instances[i]->getName();
            const string instance_name = node_def ? node_def->getNodeString() : (starts_with(name, "NG_") ? name.substr(3) : name);
            const string prefix = func_def->name() + "_";
            if (starts_with(instance_name, prefix))
            {
                const string type_name = instance_name.substr(prefix.size());
                result.insert(type_name);
                result.insert(type_alias(type_name));
            }
        }
        return result;
    }

    void DocumentDecompiler::emit_function_dependencies(const mx::ElementPtr& function)
    {
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

        GraphDecompiler body{*this, node_graph};
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

        // variables with hints are declared by their statement, which comes before the functions that use them, including
        // struct variables whose fields are nonlocal variables, e.g., x of x__0
        const mx::NodePtr node = document_->getNode(name);
        if ((node and graph_decompiler_.is_hinted(node)) or is_hinted_variable(name))
            return;

        if (contains(emitted_nonlocal_variables_, name))
            return;
        emitted_nonlocal_variables_.insert(name);

        // the value of the variable is passed to each call of the function, use the value of the first call
        ExprPtr value;
        for (const mx::NodePtr& node : document_->getNodes())
        {
            if (node->getNodeDef() != node_def)
                continue;
            const mx::InputPtr input = node->getInput(serialize::with_prefix(serialize::NONLOCAL_IN_PREFIX, name));
            if (input and input->getNodeName().empty() and input->hasValue())
            {
                value = create_literal(input->getValue());
                break;
            }
        }

        const ModifierList mods = is_mutable_variable(name) ? ModifierList{TokenType::Mutable} : ModifierList{};
        statements_.push_back(statements::create_statement<VariableDefinition>(mods, create_type_from(type_name), name, std::move(value)));
    }

    StmtPtr DocumentDecompiler::create_function_definition(const mx::NodeDefPtr& node_def)
    {
        const mx::NodeGraphPtr node_graph = implementation(node_def);

        vector<mx::OutputPtr> return_outputs;
        vector<mx::OutputPtr> out_parameter_outputs;
        AttributeList attrs = user_attributes(node_def);
        const bool is_void = is_void_function(node_def);
        for (const mx::OutputPtr& output : node_def->getActiveOutputs())
        {
            if (is_return_output(output) and not is_void)
            {
                return_outputs.push_back(output);
                attrs = concat(attrs, user_attributes(output, output->getName()));
            }
            else if (serialize::has_prefix(output->getName(), serialize::OUT_PARAMETER_PREFIX))
            {
                out_parameter_outputs.push_back(output);
            }
        }

        // parameters without a default value are declared with the default of their type, e.g., 0.0
        vector<Parameter> params;
        unordered_set<string> param_names;
        for (const mx::InputPtr& input : node_def->getActiveInputs())
        {
            if (is_implicit_input(input))
                continue;

            // ref parameters are both an input and an out parameter output
            const bool is_ref = node_def->getActiveOutput(serialize::with_prefix(serialize::OUT_PARAMETER_PREFIX, input->getName())) != nullptr;

            ExprPtr default_expr;
            if (is_ref)
                default_expr = nullptr;
            else if (not has_literal_syntax(input->getType()) or not input->hasValue())
                default_expr = create_expression<NullExpression>();
            else if (not is_zero_value(input))
                default_expr = create_literal(input->getValue());

            const string name = to_identifier(input->getName());
            param_names.insert(name);
            const ModifierList mods = is_ref ? ModifierList{TokenType::Ref} : ModifierList{};
            params.emplace_back(user_attributes(input), mods, create_type_from(input->getType()), name, std::move(default_expr), params.size());
        }

        for (const mx::OutputPtr& output : out_parameter_outputs)
        {
            if (node_def->getActiveInput(serialize::remove_prefix(output->getName())))
                continue;

            const string name = to_identifier(serialize::remove_prefix(output->getName()));
            param_names.insert(name);
            params.emplace_back(user_attributes(output), TokenType::Out, create_type_from(output->getType()), name, nullptr, params.size());
        }

        const bool use_hints = has_hinted_parameters(node_def, params);
        GraphDecompiler body{*this, node_graph, param_names, use_hints};

        vector<mx::OutputPtr> graph_outputs;
        for (const mx::OutputPtr& output : return_outputs)
            graph_outputs.push_back(node_graph->getOutput(output->getName()));
        unordered_set<mx::NodePtr> constants;
        const ReturnStatement* comptime_return = use_hints ? compile_time_return(node_def, node_graph, graph_outputs, body, constants) : nullptr;

        vector<StmtPtr> body_statements = create_body_statements(body, node_def, constants);

        for (const mx::OutputPtr& output : node_graph->getOutputs())
        {
            const string& name = output->getName();
            const bool is_out_parameter = serialize::has_prefix(name, serialize::OUT_PARAMETER_PREFIX);
            const bool is_nonlocal = serialize::has_prefix(name, serialize::NONLOCAL_OUT_PREFIX);
            if (not is_out_parameter and not is_nonlocal)
                continue;

            ExprPtr value = body.create_port_expression(output);
            if (value == nullptr)
                continue;

            const string var_name = is_out_parameter ? to_identifier(serialize::remove_prefix(name)) : serialize::remove_prefix(name);
            // the value was already assigned to the variable by the code of the body, e.g., with decompile hints
            if (value->to_string() == var_name)
                continue;
            body_statements.push_back(statements::create_statement<VariableAssignment>(create_symbol("="), create_identifier(var_name), std::move(value)));
        }

        if (comptime_return)
        {
            body_statements.push_back(statements::create_statement<ReturnStatement>(body.rename_variables(comptime_return->expression())));
        }
        else if (not return_outputs.empty())
        {
            vector<ExprPtr> return_values;
            for (const mx::OutputPtr& output : return_outputs)
            {
                const mx::OutputPtr graph_output = node_graph->getOutput(output->getName());
                ExprPtr value = graph_output ? body.create_port_expression(graph_output) : nullptr;
                return_values.push_back(value ? std::move(value) : create_expression<NullExpression>());
            }

            ExprPtr return_value = return_values.size() == 1 ? return_values.front() : create_expression<UnnamedConstructor>(return_values);
            body_statements.push_back(statements::create_statement<ReturnStatement>(std::move(return_value)));
        }

        StmtPtr func_def = statements::create_statement<FunctionDefinition>(
            ModifierList{},
            return_type(return_outputs),
            function_name(node_def),
            vector<TypePtr>{},
            ParameterList{std::move(params)},
            statements::create_statement<BlockStatement>(std::move(body_statements))
        );
        func_def->set_attributes(std::move(attrs));
        return func_def;
    }

    StmtPtr DocumentDecompiler::create_function_definition(const mx::NodeGraphPtr& node_graph)
    {
        // node graph functions have default values for all of their parameters and are called without arguments
        vector<Parameter> params;
        unordered_set<string> param_names;
        for (const mx::InputPtr& input : node_graph->getInputs())
        {
            ExprPtr default_expr = graph_decompiler_.create_port_expression(input);
            if (default_expr == nullptr)
                default_expr = create_expression<NullExpression>();

            const string name = to_identifier(input->getName());
            param_names.insert(name);
            params.emplace_back(user_attributes(input), ModifierList{}, create_type_from(input->getType()), name, std::move(default_expr), params.size());
        }

        const bool use_hints = has_hinted_parameters(node_graph, params);
        GraphDecompiler body{*this, node_graph, param_names, use_hints};

        const vector<mx::OutputPtr> outputs = node_graph->getOutputs();
        unordered_set<mx::NodePtr> constants;
        const ReturnStatement* comptime_return = use_hints ? compile_time_return(node_graph, node_graph, outputs, body, constants) : nullptr;

        vector<StmtPtr> body_statements = create_body_statements(body, node_graph, constants);

        if (comptime_return)
        {
            body_statements.push_back(statements::create_statement<ReturnStatement>(body.rename_variables(comptime_return->expression())));
        }
        else if (not outputs.empty())
        {
            vector<ExprPtr> return_values;
            for (const mx::OutputPtr& output : outputs)
            {
                ExprPtr value = body.create_port_expression(output);
                return_values.push_back(value ? std::move(value) : create_expression<NullExpression>());
            }

            ExprPtr return_value = return_values.size() == 1 ? return_values.front() : create_expression<UnnamedConstructor>(return_values);
            body_statements.push_back(statements::create_statement<ReturnStatement>(std::move(return_value)));
        }

        // parameterless functions are node graphs by default
        const bool is_parameterless = params.empty();
        StmtPtr func_def = statements::create_statement<FunctionDefinition>(
            is_parameterless ? ModifierList{} : ModifierList{TokenType::Nodegraph},
            return_type(outputs),
            function_name(node_graph),
            vector<TypePtr>{},
            is_parameterless ? std::nullopt : optional<ParameterList>{ParameterList{std::move(params)}},
            statements::create_statement<BlockStatement>(std::move(body_statements))
        );
        func_def->set_attributes(user_attributes(node_graph));
        return func_def;
    }

    bool DocumentDecompiler::has_hinted_parameters(const mx::ElementPtr& function, const vector<Parameter>& params) const
    {
        // the hints of the body refer to the parameters of the code, which are split into multiple inputs if they are
        // structs, e.g., p becomes p__x and p__y
        const optional<size_t> definition_id = DocumentHints::definition(function);
        const DocumentHints::StatementRecord* record = definition_id ? hints_.statement(*definition_id) : nullptr;
        const auto func_def = record ? std::dynamic_pointer_cast<FunctionDefinition>(record->statement) : nullptr;
        if (func_def == nullptr)
            return false;

        // the code of templated functions has the template types, e.g., `T x = v;`, so it is only used to create the
        // templated function, not its node defs as separate functions
        if (template_definition(*definition_id))
        {
            const auto it = template_functions_.find(*definition_id);
            if (it == template_functions_.end() or it->second.state == TemplateState::Separate)
                return false;
        }

        vector<string> hinted_names;
        if (func_def->parameters())
        {
            for (const Parameter& param : *func_def->parameters())
                hinted_names.push_back(param.name());
        }

        vector<string> names;
        for (const Parameter& param : params)
            names.push_back(param.name());
        return names == hinted_names;
    }

    vector<StmtPtr> DocumentDecompiler::create_body_statements(GraphDecompiler& body, const mx::ElementPtr& function, const unordered_set<mx::NodePtr>& excluded)
    {
        vector<StmtPtr> result;
        unordered_set<mx::NodePtr> emitted;
        const std::function<void(const mx::NodePtr&)> emit = [&](const mx::NodePtr& node) {
            if (contains(emitted, node) or contains(excluded, node))
                return;
            emitted.insert(node);
            for (const mx::NodePtr& dependency : body.statement_dependencies(node))
                emit(dependency);
            body.create_statements(node, result);
        };

        // statements with hints are emitted in the order of the code
        if (const optional<size_t> definition_id = DocumentHints::definition(function); definition_id and body.has_hints())
        {
            for (const size_t id : hints_.children(*definition_id))
            {
                // the return statement is created from the outputs of the node graph
                const DocumentHints::StatementRecord* record = hints_.statement(id);
                if (record and dynamic_cast<const ReturnStatement*>(record->statement.get()))
                    continue;
                if (not body.has_hinted_statement(id))
                    continue;

                for (const mx::NodePtr& node : body.hinted_statement_dependencies(id))
                    emit(node);
                body.create_hinted_statements(id, result);
            }
        }

        for (const mx::NodePtr& node : body.ordered_statements())
            emit(node);

        return result;
    }

    const ReturnStatement* DocumentDecompiler::compile_time_return(const mx::ElementPtr& function, const mx::NodeGraphPtr& node_graph, const vector<mx::OutputPtr>& outputs, const GraphDecompiler& body, unordered_set<mx::NodePtr>& constants) const
    {
        const optional<size_t> definition_id = DocumentHints::definition(function);
        if (not definition_id or outputs.empty())
            return nullptr;

        // the function has a single return statement, whose value is a compile-time value, e.g., `return 1.0;`
        const ReturnStatement* result = nullptr;
        for (const size_t id : hints_.children(*definition_id))
        {
            const DocumentHints::StatementRecord* record = hints_.statement(id);
            const auto return_stmt = record ? dynamic_cast<const ReturnStatement*>(record->statement.get()) : nullptr;
            if (return_stmt == nullptr)
                continue;
            if (result or not record->is_complete or return_stmt->expression() == nullptr)
                return nullptr;
            result = return_stmt;
        }
        if (result == nullptr)
            return nullptr;

        // compile-time values are written to the outputs as values, or as constant nodes that are only used by the output
        unordered_set<mx::NodePtr> output_constants;
        for (const mx::OutputPtr& output : outputs)
        {
            if (output == nullptr)
                return nullptr;
            if (output->getNodeName().empty())
            {
                if (output->getInterfaceName().empty() and output->hasValue())
                    continue;
                return nullptr;
            }

            const mx::NodePtr node = node_graph->getNode(output->getNodeName());
            const mx::InputPtr value = node ? node->getInput("value") : nullptr;
            if (node == nullptr or node->getCategory() != "constant" or body.is_hinted(node) or body.use_count(node) != 1 or
                value == nullptr or is_connected(value) or not value->hasValue())
                return nullptr;
            output_constants.insert(node);
        }

        constants.insert(output_constants.begin(), output_constants.end());
        return result;
    }

    string DocumentDecompiler::print() const
    {
        if (statements_.empty())
            return "";
        return join_statements(statements_) + "\n";
    }
}
