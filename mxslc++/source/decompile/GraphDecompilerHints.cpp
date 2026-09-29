//
// Created by jaket on 28/09/2026.
//

// The parts of GraphDecompiler that recreate code from decompile hints, see serialize/decompile_hints.h.
//
// Statements with hints are created from their skeleton, i.e., the statement recorded by the compiler without the parts
// that are recovered from the graph. Calls to inline functions are collapsed back into a single call, loops are created
// again if all of their iterations create the same code, and variables are named after the variables that held them.

#include "decompile/GraphDecompiler.h"

#include "decompile/DocumentDecompiler.h"
#include "decompile/decompile_utils.h"
#include "expressions/CompoundAssignment.h"
#include "expressions/DotOperator.h"
#include "expressions/ExpressionFactory.h"
#include "expressions/FunctionCall.h"
#include "expressions/Identifier.h"
#include "expressions/IncrementOperator.h"
#include "expressions/IndexingOperator.h"
#include "expressions/Literal.h"
#include "expressions/NamedConstructor.h"
#include "expressions/RangeExpression.h"
#include "expressions/UnnamedConstructor.h"
#include "expressions/VariableDefinitionExpression.h"
#include "expressions/interface.h"
#include "parse.h"
#include "runtime/ArgumentList.h"
#include "runtime/Field.h"
#include "runtime/Parameter.h"
#include "runtime/ParameterList.h"
#include "runtime/Type.h"
#include "runtime/interface.h"
#include "scan.h"
#include "serialize/decompile_hints.h"
#include "serialize/serialize_name_utils.h"
#include "statements/BlockStatement.h"
#include "statements/ExpressionStatement.h"
#include "statements/ForEachLoop.h"
#include "statements/FunctionDefinition.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/ReturnStatement.h"
#include "statements/UsingDeclaration.h"
#include "statements/VariableAssignment.h"
#include "statements/VariableDefinition.h"
#include "statements/interface.h"
#include "utils/container_utils.h"

namespace mxslc::decompile
{
    using namespace decompile_utils;
    using container_utils::contains;
    namespace hints = serialize::hints;

    namespace
    {
        const unordered_map<string, string>& binary_dunders()
        {
            static const unordered_map<string, string> dunders {
                {"__add__", "+"}, {"__sub__", "-"}, {"__mul__", "*"}, {"__div__", "/"}, {"__mod__", "%"}, {"__pow__", "^"},
                {"__eq__", "=="}, {"__ne__", "!="}, {"__gt__", ">"}, {"__lt__", "<"}, {"__ge__", ">="}, {"__le__", "<="},
                {"__and__", "&"}, {"__or__", "|"},
            };
            return dunders;
        }

        const unordered_map<string, string>& unary_dunders()
        {
            static const unordered_map<string, string> dunders {{"__pos__", "+"}, {"__neg__", "-"}, {"__not__", "!"}};
            return dunders;
        }

        const unordered_map<string, string>& compound_dunders()
        {
            static const unordered_map<string, string> dunders {
                {"+=", "__add__"}, {"-=", "__sub__"}, {"*=", "__mul__"}, {"/=", "__div__"}, {"%=", "__mod__"}, {"^=", "__pow__"},
                {"&=", "__and__"}, {"|=", "__or__"},
            };
            return dunders;
        }

        // the constructors of stdlib.mxsl, e.g., __vector3__ -> vec3
        optional<string> constructor_type(const string& function)
        {
            static const unordered_set<string> types {
                "boolean", "integer", "float", "vector2", "vector3", "vector4", "color3", "color4", "matrix33", "matrix44"
            };
            if (function.size() < 5 or function.substr(0, 2) != "__" or function.substr(function.size() - 2) != "__")
                return std::nullopt;
            const string type_name = function.substr(2, function.size() - 4);
            if (not contains(types, type_name))
                return std::nullopt;
            return type_alias(type_name);
        }

        string value_key(const mx::NodePtr& node, const string& output_name)
        {
            // single output nodes are bound without an output name
            if (node->getType() != mx::MULTI_OUTPUT_TYPE_STRING)
                return node->getName() + ".";
            return node->getName() + "." + output_name;
        }

        bool is_placeholder(const ExprPtr& expr)
        {
            const IdentifierPtr identifier = cast_expression<Identifier>(expr);
            return identifier and identifier->name() == hints::PLACEHOLDER;
        }

        // a copy of a statement from its skeleton, the parsed skeletons of the records are shared
        StmtPtr parse_statement(const string& skeleton)
        {
            try
            {
                Parser parser{DocumentHints::scan_skeleton(skeleton)};
                return parser.statement();
            }
            catch (...)
            {
                return nullptr;
            }
        }

        // the variable at the root of an assignment, e.g., q for `q.xy`
        IdentifierPtr root_identifier(const ExprPtr& lvalue)
        {
            if (const IdentifierPtr identifier = cast_expression<Identifier>(lvalue))
                return identifier;
            if (const DotOperatorPtr dot = cast_expression<DotOperator>(lvalue))
                return root_identifier(dot->value_expression());
            if (const IndexingOperatorPtr indexing = cast_expression<IndexingOperator>(lvalue))
                return root_identifier(indexing->value_expression());
            return nullptr;
        }

        // the lvalue with its root variable renamed, e.g., if the variable was renamed because it was declared twice
        ExprPtr rename_root(const ExprPtr& lvalue, const string& identifier)
        {
            if (cast_expression<Identifier>(lvalue))
                return create_identifier(identifier);
            if (const DotOperatorPtr dot = cast_expression<DotOperator>(lvalue))
                return create_expression<DotOperator>(rename_root(dot->value_expression(), identifier), dot->token());
            return lvalue;
        }

        // an argument of a call record, e.g., `out fragColor`
        optional<Argument> parse_argument(const string& code)
        {
            try
            {
                Parser parser{DocumentHints::scan_skeleton("f(" + code + ")")};
                const FunctionCallPtr call = cast_expression<FunctionCall>(parser.expression());
                if (call and call->arguments().size() == 1)
                    return *call->arguments().begin();
            }
            catch (...)
            {
            }
            return std::nullopt;
        }

        // the fields of an assignment, e.g., {"origin", "x"} for `ray.origin.x`, if they are known
        optional<vector<string>> field_path(const ExprPtr& lvalue)
        {
            if (cast_expression<Identifier>(lvalue))
                return vector<string>{};
            if (const DotOperatorPtr dot = cast_expression<DotOperator>(lvalue))
            {
                optional<vector<string>> path = field_path(dot->value_expression());
                if (path)
                    path->push_back(dot->token().lexeme());
                return path;
            }
            if (const IndexingOperatorPtr indexing = cast_expression<IndexingOperator>(lvalue))
            {
                // the index is only known if it is an integer literal, e.g., `v[1]`
                const vector<Token> tokens = DocumentHints::scan_skeleton(indexing->to_string());
                const size_t count = tokens.size();
                if (count < 4 or tokens[count - 1] != ']' or tokens[count - 2] != TokenType::Int or tokens[count - 3] != '[')
                    return std::nullopt;

                optional<vector<string>> path = field_path(indexing->value_expression());
                if (path)
                    path->push_back(tokens[count - 2].lexeme());
                return path;
            }
            return std::nullopt;
        }

        // the types of multi-variable definitions are recorded with their MaterialX names, e.g., vector4 for vec4
        TypePtr declared_type_alias(const TypePtr& type)
        {
            if (type == nullptr or type->has_fields())
                return type;
            const string name = type->to_string();
            return type_alias(name) != name ? create_type_from(name) : type;
        }

        bool is_index(const string& name)
        {
            return not name.empty() and std::all_of(name.begin(), name.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
        }

        bool is_same_output(const string& a, const string& b)
        {
            return a == b or a.empty() or b.empty();
        }
    }

    bool GraphDecompiler::is_hinted(const mx::NodePtr& node) const
    {
        return hinted_ and hinted_->instance(node) != nullptr;
    }

    bool GraphDecompiler::is_hinted_variable(const string& name) const
    {
        return std::any_of(state_.scopes.begin(), state_.scopes.end(), [&](const HintScope& scope) {
            return contains(scope.variables, name);
        });
    }

    bool GraphDecompiler::has_hinted_statement(const size_t id) const
    {
        if (not hinted_)
            return false;

        const DocumentHints::StatementRecord* record = hinted_->hints().statement(id);
        if (record == nullptr or record->statement == nullptr or dynamic_cast<const FunctionDefinition*>(record->statement.get()))
            return false;

        if (hinted_->is_removed(id))
            return false;
        if (hinted_->statement(hinted_->root(), id) != nullptr or contains(port_assignments_, id) or record->is_complete)
            return true;

        // statements that assign values that were created before them, e.g., `float y = x;`
        const statements::Statement* stmt = record->statement.get();
        const auto var_def = dynamic_cast<const VariableDefinition*>(stmt);
        const auto assignment = dynamic_cast<const VariableAssignment*>(stmt);
        const IdentifierPtr root = assignment ? root_identifier(assignment->lhs_expression()) : nullptr;
        const string var_name = var_def ? var_def->name() : root ? root->name() : "";
        return not var_name.empty() and not hinted_->alias_values(id, hinted_->root(), var_name).empty();
    }

    void GraphDecompiler::create_hinted_statements(const size_t id, vector<StmtPtr>& result)
    {
        if (not hinted_)
            return;
        create_hinted_statement(id, hinted_->statement(hinted_->root(), id), hinted_->root(), result);
    }

    vector<mx::NodePtr> GraphDecompiler::hinted_statement_dependencies(const size_t id) const
    {
        const Instance* instance = hinted_ ? hinted_->statement(hinted_->root(), id) : nullptr;
        if (instance == nullptr)
            return {};

        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited;
        for (const mx::NodePtr& node : hinted_->nodes(*instance))
        {
            for (const mx::InputPtr& input : node->getInputs())
                collect_dependencies(input, statements, functions, visited);
        }
        return statements;
    }

    vector<mx::ElementPtr> GraphDecompiler::hinted_function_dependencies(const size_t id) const
    {
        const Instance* instance = hinted_ ? hinted_->statement(hinted_->root(), id) : nullptr;
        if (instance == nullptr)
            return {};

        vector<mx::NodePtr> statements;
        vector<mx::ElementPtr> functions;
        unordered_set<mx::NodePtr> visited;
        for (const mx::NodePtr& node : hinted_->nodes(*instance))
            collect_dependencies(node, statements, functions, visited);
        return functions;
    }

    StmtPtr GraphDecompiler::create_inline_function(const size_t definition_id)
    {
        const DocumentHints::StatementRecord* record = hinted_->hints().statement(definition_id);
        const auto func_def = record ? std::dynamic_pointer_cast<FunctionDefinition>(record->statement) : nullptr;
        if (func_def == nullptr)
            return nullptr;

        // the body is recreated from the calls of the function, or from its skeletons if its calls did not create nodes
        vector<StmtPtr> body;
        if (const auto it = inline_function_bodies_.find(definition_id); it != inline_function_bodies_.end())
        {
            body = std::move(it->second);
            inline_function_bodies_.erase(it);
        }
        else if (not create_skeleton_body(definition_id, body))
        {
            return nullptr;
        }

        return func_def->with_body(statements::create_statement<BlockStatement>(std::move(body)));
    }

    bool GraphDecompiler::create_skeleton_body(const size_t definition_id, vector<StmtPtr>& result)
    {
        const DocumentHints::StatementRecord* definition = hinted_->hints().statement(definition_id);
        const auto func_def = definition ? std::dynamic_pointer_cast<FunctionDefinition>(definition->statement) : nullptr;
        if (func_def == nullptr)
            return false;

        const HintState saved_state = state_;
        state_ = HintState{};
        state_.is_function_body = true;
        if (func_def->parameters())
        {
            for (const Parameter& param : *func_def->parameters())
            {
                state_.scopes.back().variables[param.name()] = VariableInfo{param.name()};
                state_.scopes.back().identifiers.insert(param.name());
            }
        }

        bool is_complete = true;
        vector<StmtPtr> statements;
        for (const size_t id : hinted_->hints().children(definition_id))
        {
            const DocumentHints::StatementRecord* record = hinted_->hints().statement(id);
            is_complete = is_complete and record and create_record_statement(*record, statements);
        }

        state_ = saved_state;
        if (is_complete)
            result = std::move(statements);
        return is_complete;
    }

    void GraphDecompiler::find_assigned_inputs()
    {
        for (const mx::NodePtr& node : nodes_)
        {
            for (const mx::InputPtr& input : node->getInputs())
            {
                const optional<size_t> id = DocumentHints::assignment(input);
                if (not id)
                    continue;

                // the assignment is only created if its statement is still part of the graph
                const DocumentHints::StatementRecord* record = hinted_->hints().statement(*id);
                const auto assignment = record ? std::dynamic_pointer_cast<VariableAssignment>(record->statement) : nullptr;
                const auto lhs = assignment ? cast_expression<DotOperator>(assignment->lhs_expression()) : nullptr;
                // assignments in the body of a function or loop are part of the arguments of the call
                const bool is_top_level = record and record->parent == root_definition_;
                if (lhs and is_top_level and lhs->token().lexeme() == input->getName() and not contains(port_assignments_, *id))
                {
                    assigned_inputs_.insert(input);
                    port_assignments_[*id] = input;
                }
            }
        }
    }

    void GraphDecompiler::remove_unnamed_multi_outputs()
    {
        // the outputs of multi-output nodes can only be referenced through a variable that holds the node, so nodes that
        // are not held by a variable, a call or the pattern of another node are created without hints
        for (const mx::NodePtr& node : nodes_)
        {
            if (not is_hinted(node) or node->getType() != mx::MULTI_OUTPUT_TYPE_STRING or contains(absorbed_, node))
                continue;

            // or they are hidden by a call to a library function, e.g., the separate node of `v[i] = x;`
            bool is_hidden = false;
            for (const Instance* instance = hinted_->instance(node); instance; instance = instance->parent)
            {
                const DocumentHints::CallRecord* call = instance->is_call() ? hinted_->hints().call(instance->frame.id) : nullptr;
                is_hidden = is_hidden or (call and call->definition == 0 and hinted_->is_collapsed(*instance));
            }
            if (is_hidden)
                continue;

            const vector<DocumentHints::Binding> bindings = DocumentHints::bindings(node);
            const bool is_bound = std::any_of(bindings.begin(), bindings.end(), [](const DocumentHints::Binding& binding) {
                return binding.frame.kind == 's';
            });

            // the separate nodes of swizzle assignments are part of the assignment, e.g., `q.xz = p.zx;`
            const Instance* statement = hinted_->instance(node);
            while (statement and not statement->is_statement())
                statement = statement->parent;
            const DocumentHints::StatementRecord* record = statement ? hinted_->hints().statement(statement->frame.id) : nullptr;
            const auto assignment = record ? std::dynamic_pointer_cast<VariableAssignment>(record->statement) : nullptr;
            const bool is_swizzle_assignment = assignment and cast_expression<DotOperator>(assignment->lhs_expression()) and is_connected(node->getInput("in"));

            // or the return value of a collapsed call, e.g., `float x, y = separate2(v);`
            const bool is_return_value = std::any_of(bindings.begin(), bindings.end(), [&](const DocumentHints::Binding& binding) {
                if (binding.frame.kind != 'c')
                    return false;
                for (const Instance* instance = hinted_->instance(node); instance; instance = instance->parent)
                {
                    if (instance->frame == binding.frame)
                        return hinted_->is_collapsed(*instance);
                }
                return false;
            });

            // nodes that are used once are called where they are used, e.g., `separate2(v).outx`
            if (not is_bound and not is_swizzle_assignment and not is_return_value and use_count(node) > 1)
                hinted_->remove_statement(node);
        }
    }

    void GraphDecompiler::declare_out_arguments(const mx::NodePtr& node)
    {
        for (const mx::OutputPtr& output : out_parameter_outputs(node))
        {
            string identifier;
            for (const DocumentHints::Binding& binding : DocumentHints::bindings(node))
            {
                if (level_ and binding.frame == level_->frame and binding.output == output->getName() and binding.path.size() == 1)
                    identifier = declare_variable(binding.path.front());
            }

            if (identifier.empty())
                identifier = output_identifier(node, output->getName());
            else
                output_identifiers_[node->getName() + "." + output->getName()] = identifier;

            state_.value_names[value_key(node, output->getName())] = {Holder{identifier, {}}};
        }
    }

    ExprPtr GraphDecompiler::create_multi_output_expression(const vector<HintedGraph::BoundValue>& values)
    {
        // a struct whose fields are all of the outputs of a node is the value of the node call, e.g., `Ray r = ray();`
        if (values.empty())
            return nullptr;

        // or all of the fields of the return value of an inline function, e.g., `float f, color3 c = foo();`
        const Instance* common_call = nullptr;
        vector<string> fields;
        for (const HintedGraph::BoundValue& value : values)
        {
            const Instance* call = hinted_->hiding_call(value.value.node, level_);
            if (call == nullptr or (common_call and call != common_call))
            {
                common_call = nullptr;
                break;
            }
            common_call = call;
            for (const HintedGraph::BoundValue& call_value : hinted_->call_values(*call))
            {
                if (call_value.value == value.value and call_value.path.size() == 1)
                    fields.push_back(call_value.path.front());
            }
        }
        if (common_call and fields.size() == values.size())
        {
            // the fields of the return value, which excludes out parameters
            const DocumentHints::CallRecord* record = hinted_->hints().call(common_call->frame.id);
            unordered_set<string> return_fields;
            for (const HintedGraph::BoundValue& call_value : hinted_->call_values(*common_call))
            {
                const bool is_out_param = record and std::any_of(record->arguments.begin(), record->arguments.end(), [&](const DocumentHints::Argument& arg) {
                    return arg.param == call_value.path.front();
                });
                if (call_value.path.size() == 1 and not is_out_param)
                    return_fields.insert(call_value.path.front());
            }

            bool is_in_order = return_fields.size() == fields.size();
            for (size_t i = 0; is_in_order and i < fields.size(); ++i)
                is_in_order = not is_index(fields[i]) or fields[i] == std::to_string(i);
            if (is_in_order)
                return create_call_expression(*common_call);
        }

        const mx::NodePtr& node = values.front().value.node;
        if (node->getType() != mx::MULTI_OUTPUT_TYPE_STRING)
            return nullptr;

        unordered_set<string> outputs;
        for (const HintedGraph::BoundValue& value : values)
        {
            if (value.value.node != node or value.path.size() != 1 or contains(outputs, value.value.output))
                return nullptr;
            outputs.insert(value.value.output);
        }
        if (outputs.size() != return_outputs(node).size())
            return nullptr;

        if (const Instance* call = hinted_->hiding_call(node, level_))
            return create_call_expression(*call);
        return create_node_expression(node);
    }

    void GraphDecompiler::find_inline_functions()
    {
        // calls to inline functions that do not create the same code as the other calls of the function are dissolved,
        // which changes the code of the functions that call them, so this is repeated until nothing changes
        std::map<size_t, vector<const Instance*>> calls;
        bool is_changed = true;
        while (is_changed)
        {
            is_changed = false;
            calls.clear();

            std::function<void(const Instance&)> collect = [&](const Instance& instance) {
                for (const Instance* child : instance.children)
                {
                    const DocumentHints::CallRecord* record = child->is_call() ? hinted_->hints().call(child->frame.id) : nullptr;
                    if (record and record->definition > 0 and hinted_->is_collapsed(*child))
                    {
                        // inline functions are only defined in the document, the calls in other graphs must match them
                        if (graph_->isA<mx::Document>() or document_.is_inline_function_defined(record->definition))
                            calls[record->definition].push_back(child);
                        else
                            hinted_->dissolve(*child);
                    }
                    collect(*child);
                }
            };
            collect(hinted_->root());

            for (const auto& [definition, instances] : calls)
            {
                vector<string> bodies;
                for (const Instance* call : instances)
                    bodies.push_back(print_call_body(*call, nullptr));

                // the body that most calls create
                string majority;
                size_t majority_count = 0;
                for (const string& body : bodies)
                {
                    const size_t count = std::count(bodies.begin(), bodies.end(), body);
                    if (count > majority_count)
                    {
                        majority = body;
                        majority_count = count;
                    }
                }

                for (size_t i = 0; i < instances.size(); ++i)
                {
                    if (bodies[i] != majority)
                    {
                        hinted_->dissolve(*instances[i]);
                        is_changed = true;
                    }
                }
            }
        }

        if (not graph_->isA<mx::Document>())
            return;

        for (const auto& [definition, instances] : calls)
        {
            vector<StmtPtr> body;
            print_call_body(*instances.front(), &body);
            inline_function_bodies_[definition] = std::move(body);
        }
    }

    string GraphDecompiler::print_call_body(const Instance& call, vector<StmtPtr>* body)
    {
        const DocumentHints::CallRecord* record = hinted_->hints().call(call.frame.id);
        const DocumentHints::StatementRecord* definition = record ? hinted_->hints().statement(record->definition) : nullptr;
        const auto func_def = definition ? std::dynamic_pointer_cast<FunctionDefinition>(definition->statement) : nullptr;
        if (func_def == nullptr)
            return "";

        const HintState saved_state = state_;
        const Instance* saved_level = level_;
        const Instance* saved_call = body_call_;

        // the body only has access to the parameters of the function, which keep their names
        state_ = HintState{};
        state_.is_function_body = true;
        body_call_ = &call;
        if (func_def->parameters())
        {
            for (const Parameter& param : *func_def->parameters())
            {
                state_.scopes.back().variables[param.name()] = VariableInfo{param.name()};
                state_.scopes.back().identifiers.insert(param.name());
            }
        }

        vector<StmtPtr> statements;
        create_region(hinted_->hints().children(record->definition), call, statements);
        const string result = join_statements(statements);

        state_ = saved_state;
        level_ = saved_level;
        body_call_ = saved_call;

        if (body)
            *body = std::move(statements);
        return result;
    }

    void GraphDecompiler::create_region(const vector<size_t>& ids, const Instance& parent, vector<StmtPtr>& result)
    {
        for (const size_t id : ids)
            create_hinted_statement(id, hinted_->statement(parent, id), parent, result);
    }

    void GraphDecompiler::create_hinted_statement(const size_t id, const Instance* instance, const Instance& parent, vector<StmtPtr>& result)
    {
        const DocumentHints::StatementRecord* record = hinted_->hints().statement(id);
        if (record == nullptr or record->statement == nullptr)
            return;

        // port assignments do not create nodes, they set the input of a node that was already created
        const auto port_assignment = std::dynamic_pointer_cast<VariableAssignment>(record->statement);
        if (port_assignment and contains(port_assignments_, id))
        {
            create_port_assignment(*port_assignment, port_assignments_.at(id), result);
            return;
        }

        // return statements can return values created by other statements, e.g., `return {x, y};`
        if (dynamic_cast<const ReturnStatement*>(record->statement.get()))
        {
            const Instance* saved_level = level_;
            level_ = instance ? instance : body_call_;
            create_return(*record, result);
            level_ = saved_level;
            return;
        }

        if (instance == nullptr)
        {
            if (not hinted_->is_removed(id) and not create_record_statement(*record, result))
                create_alias_statement(*record, parent, result);
            return;
        }

        const Instance* saved_level = level_;
        level_ = instance;

        const statements::Statement* stmt = record->statement.get();
        if (not dynamic_cast<const ForEachLoop*>(stmt))
            create_shared_values(*instance, result);

        if (const auto var_def = dynamic_cast<const VariableDefinition*>(stmt))
            create_variable_definition(*var_def, *instance, result);
        else if (const auto multi_var_def = dynamic_cast<const MultiVariableDefinition*>(stmt))
            create_multi_variable_definition(*multi_var_def, *instance, result);
        else if (const auto assignment = dynamic_cast<const VariableAssignment*>(stmt))
            create_variable_assignment(*assignment, *instance, result);
        else if (const auto expr_stmt = dynamic_cast<const ExpressionStatement*>(stmt))
            create_expression_statement(*expr_stmt, *instance, result);
        else if (const auto loop = dynamic_cast<const ForEachLoop*>(stmt))
            create_loop(*loop, *instance, result);

        // copies of values that are used after their variable is assigned a new value, e.g., `float x2 = x;`
        for (const auto& [key, value] : std::exchange(pending_snapshots_, {}))
        {
            const auto& [identifier, path] = state_.value_names.at(key).back();
            const mx::OutputPtr output = value.output.empty() ? nullptr : value.node->getOutput(value.output);
            const string type_name = output ? output->getType() : value.node->getType();
            const string snapshot = declare_variable(identifier + "_copy");
            result.push_back(statements::create_statement<VariableDefinition>(ModifierList{}, create_type_from(type_name), snapshot, variable_expression(identifier, path)));
            state_.snapshot_names[key] = snapshot;
        }

        level_ = saved_level;
    }

    bool GraphDecompiler::create_alias_statement(const DocumentHints::StatementRecord& record, const Instance& parent, vector<StmtPtr>& result)
    {
        if (const auto var_def = dynamic_cast<const VariableDefinition*>(record.statement.get()))
        {
            const vector<HintedGraph::BoundValue> values = hinted_->alias_values(record.id, parent, var_def->name());
            if (values.empty())
                return false;

            const bool is_single_value = values.size() == 1 and values.front().path.empty();
            ExprPtr expr = is_single_value ? create_value_expression(values.front().value) : create_struct_expression(values, resolve_type(var_def->declared_type()), var_def->expression());
            if (expr == nullptr)
                return false;

            const string identifier = declare_variable(var_def->name());
            result.push_back(statements::create_statement<VariableDefinition>(var_def->modifiers(), var_def->declared_type(), identifier, std::move(expr)));
            assign_values(identifier, values, /*is_definition*/true);
            return true;
        }

        if (const auto assignment = dynamic_cast<const VariableAssignment*>(record.statement.get()))
        {
            const IdentifierPtr root = root_identifier(assignment->lhs_expression());
            VariableInfo* var = root ? find_variable(root->name()) : nullptr;
            const optional<vector<string>> lhs_path = field_path(assignment->lhs_expression());
            if (var == nullptr or not lhs_path)
                return false;

            // the whole variable is bound, only the values of the assigned field were assigned, e.g., p.z of `p.z = p.x;`
            vector<HintedGraph::BoundValue> values;
            for (const HintedGraph::BoundValue& value : hinted_->alias_values(record.id, parent, root->name()))
            {
                if (value.path.size() >= lhs_path->size() and std::equal(lhs_path->begin(), lhs_path->end(), value.path.begin()))
                    values.push_back(value);
            }
            if (values.size() != 1 or values.front().path != *lhs_path)
                return false;

            ExprPtr rhs = create_value_expression(values.front().value);
            if (rhs == nullptr)
                return false;

            result.push_back(statements::create_statement<VariableAssignment>(create_symbol("="), rename_variables(assignment->lhs_expression()), std::move(rhs)));
            assign_values(var->identifier, values, /*is_definition*/false);
            var->statement.reset();
            return true;
        }

        return false;
    }

    void GraphDecompiler::create_shared_values(const Instance& instance, vector<StmtPtr>& result)
    {
        for (const mx::NodePtr& node : nodes_)
        {
            const Instance* node_instance = hinted_->instance(node);
            if (node_instance == nullptr or not hinted_->is_within(*node_instance, instance))
                continue;
            if (contains(absorbed_, node) or node->getType() == mx::MULTI_OUTPUT_TYPE_STRING or hinted_->hiding_call(node, &instance))
                continue;
            if (contains(state_.value_names, value_key(node, "")))
                continue;

            // a collapsed call is a single use of its arguments, even if it uses them more than once
            size_t use_count = 0;
            unordered_set<const Instance*> calls;
            for (const Use& use : contains(uses_, node) ? uses_.at(node) : vector<Use>{})
            {
                const mx::NodePtr consumer = use.consumer->asA<mx::Node>();
                const Instance* consumer_instance = consumer ? hinted_->instance(consumer) : nullptr;
                if (consumer_instance == nullptr or not hinted_->is_within(*consumer_instance, instance))
                    continue;
                if (const Instance* call = hinted_->hiding_call(consumer, &instance))
                    use_count += calls.insert(call).second ? 1 : 0;
                else
                    ++use_count;
            }
            if (use_count <= 1)
                continue;

            ExprPtr expr = create_node_expression(node);
            const string identifier = declare_variable(is_temporary_name(node->getName()) ? "var_" + node->getName().substr(5) : node->getName());
            result.push_back(statements::create_statement<VariableDefinition>(ModifierList{}, create_type_from(node->getType()), identifier, std::move(expr)));
            state_.value_names[value_key(node, "")] = {Holder{identifier, {}}};
        }
    }

    bool GraphDecompiler::create_record_statement(const DocumentHints::StatementRecord& record, vector<StmtPtr>& result)
    {
        // statements without nodes can only be created if they are complete, e.g., `float PI = 3.14159;`
        const statements::Statement* stmt = record.statement.get();
        if (not record.is_complete or dynamic_cast<const FunctionDefinition*>(stmt) or dynamic_cast<const ForEachLoop*>(stmt))
            return false;

        // the code of the skeleton references the variables by their names in the source code
        if (const auto var_def = dynamic_cast<const VariableDefinition*>(stmt))
        {
            // the value is renamed before the variable is declared, which can shadow a variable, e.g., `float x = x;`
            ExprPtr value = rename_variables(var_def->expression());
            const string identifier = declare_variable(var_def->name(), record.id);
            result.push_back(statements::create_statement<VariableDefinition>(var_def->modifiers(), var_def->declared_type(), identifier, std::move(value)));
            return true;
        }

        if (const auto multi_var_def = dynamic_cast<const MultiVariableDefinition*>(stmt))
        {
            ExprPtr value = rename_variables(multi_var_def->expression());
            vector<Field> fields;
            for (const Field& field : multi_var_def->declared_type()->fields())
                fields.emplace_back(field.modifiers(), declared_type_alias(field.type()), declare_variable(field.name(), record.id), /*is_multi_var_def*/true);
            result.push_back(statements::create_statement<MultiVariableDefinition>(create_type(std::move(fields)), std::move(value)));
            return true;
        }

        if (const auto assignment = dynamic_cast<const VariableAssignment*>(stmt))
        {
            const IdentifierPtr root = root_identifier(assignment->lhs_expression());
            VariableInfo* var = root ? find_variable(root->name()) : nullptr;
            if (var == nullptr)
                return false;

            ExprPtr lhs = rename_variables(assignment->lhs_expression());
            ExprPtr rhs = rename_variables(assignment->rhs_expression());
            result.push_back(statements::create_statement<VariableAssignment>(create_symbol("="), std::move(lhs), std::move(rhs)));

            // a compile-time value is assigned to the variable, so it no longer holds a node output
            assign_values(var->identifier, {}, /*is_definition*/false);
            var->statement = record.id;
            return true;
        }

        if (const auto expr_stmt = dynamic_cast<const ExpressionStatement*>(stmt))
        {
            // e.g., `x += 1.0;`, whose skeleton is also complete if x holds nodes, e.g., `x++;`
            const ExprPtr lvalue = expr_stmt->assigned_expression();
            const IdentifierPtr root = lvalue ? root_identifier(lvalue) : nullptr;
            VariableInfo* var = root ? find_variable(root->name()) : nullptr;
            if (var == nullptr or not var->statement)
                return false;

            result.push_back(statements::create_statement<ExpressionStatement>(rename_variables(expr_stmt->expression())));
            var->statement = record.id;
            return true;
        }

        if (const auto return_stmt = dynamic_cast<const ReturnStatement*>(stmt))
        {
            result.push_back(statements::create_statement<ReturnStatement>(rename_variables(return_stmt->expression())));
            return true;
        }

        if (dynamic_cast<const UsingDeclaration*>(stmt))
        {
            if (StmtPtr copy = parse_statement(record.skeleton))
            {
                result.push_back(std::move(copy));
                return true;
            }
        }

        return false;
    }

    void GraphDecompiler::create_variable_definition(const VariableDefinition& var_def, const Instance& instance, vector<StmtPtr>& result)
    {
        const string& name = var_def.name();
        const vector<HintedGraph::BoundValue> values = hinted_->statement_values(instance, name);

        if (values.empty())
        {
            // variables without nodes, e.g., compile-time values, are only created from complete skeletons
            const DocumentHints::StatementRecord* record = hinted_->hints().statement(instance.frame.id);
            if (record)
                create_record_statement(*record, result);
            return;
        }

        ExprPtr expr;
        if (var_def.modifiers().contains(TokenType::Geomprop))
        {
            // `geomprop float x = 1.0;` is compiled to `geompropvalue("x", 1.0)`
            const mx::InputPtr default_input = values.front().value.node->getInput("default");
            expr = default_input ? create_port_expression(default_input) : nullptr;
        }
        else if (values.size() == 1 and values.front().path.empty())
        {
            // calls with out arguments declare their variables, e.g., `float c = sincos(x, float s);`
            const mx::NodePtr& node = values.front().value.node;
            if (not out_parameter_outputs(node).empty() and hinted_->hiding_call(node, level_) == nullptr)
            {
                declare_out_arguments(node);
                expr = create_function_call(node, /*with_out_arguments*/true);
            }
            else
            {
                expr = create_value_expression(values.front().value);
            }
        }
        else
        {
            expr = create_struct_expression(values, resolve_type(var_def.declared_type()), var_def.expression());
        }

        // variables are named after their node if it was renamed after it was compiled
        string preferred_identifier;
        if (values.size() == 1)
        {
            const mx::NodePtr& node = values.front().value.node;
            const string& compiled_name = node->getAttribute(hints::NAME_ATTRIBUTE);
            if (not compiled_name.empty() and compiled_name != node->getName() and is_valid_identifier(node->getName()))
                preferred_identifier = node->getName();
        }

        const string identifier = declare_variable(name, std::nullopt, preferred_identifier);
        StmtPtr stmt = statements::create_statement<VariableDefinition>(var_def.modifiers(), var_def.declared_type(), identifier, std::move(expr));
        if (values.size() == 1)
            stmt->set_attributes(user_attributes(values.front().value.node));
        result.push_back(std::move(stmt));

        assign_values(identifier, values, /*is_definition*/true);
    }

    void GraphDecompiler::create_multi_variable_definition(const MultiVariableDefinition& var_def, const Instance& instance, vector<StmtPtr>& result)
    {
        const TypePtr& type = var_def.declared_type();

        // `float x, y = separate2(v);` binds each variable to an output of the same node
        vector<vector<HintedGraph::BoundValue>> field_values;
        vector<HintedGraph::BoundValue> struct_values;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            field_values.push_back(hinted_->statement_values(instance, type->field_name(i)));
            const vector<HintedGraph::BoundValue>& values = field_values.back();
            if (values.size() == 1 and values.front().path.empty())
                struct_values.push_back(HintedGraph::BoundValue{values.front().value, {type->field_name(i)}});
        }

        // `geomprop vec2 a, b = {x, y};` is compiled to geompropvalue nodes, whose default values are x and y
        const vector<Field>& declared_fields = type->fields();
        const bool is_geomprop = std::all_of(declared_fields.begin(), declared_fields.end(), [](const Field& field) {
            return field.modifiers().contains(TokenType::Geomprop);
        });

        ExprPtr expr;
        if (is_geomprop)
        {
            if (var_def.expression() and not is_placeholder(var_def.expression()))
            {
                expr = rename_variables(var_def.expression());
            }
            else if (var_def.expression())
            {
                vector<ExprPtr> defaults;
                for (const vector<HintedGraph::BoundValue>& values : field_values)
                {
                    const mx::InputPtr default_input = values.size() == 1 ? values.front().value.node->getInput("default") : nullptr;
                    defaults.push_back(default_input ? create_port_expression(default_input) : nullptr);
                }
                if (std::any_of(defaults.begin(), defaults.end(), [](const ExprPtr& e) { return e == nullptr; }))
                    return;
                expr = create_expression<UnnamedConstructor>(std::move(defaults));
            }
        }
        else if (struct_values.size() == type->field_count())
        {
            expr = create_multi_output_expression(struct_values);
        }

        if (expr == nullptr and not is_geomprop)
        {
            vector<ExprPtr> field_exprs;
            for (const vector<HintedGraph::BoundValue>& values : field_values)
                field_exprs.push_back(values.size() == 1 ? create_value_expression(values.front().value) : nullptr);
            if (std::any_of(field_exprs.begin(), field_exprs.end(), [](const ExprPtr& e) { return e == nullptr; }))
                return;
            expr = create_expression<UnnamedConstructor>(std::move(field_exprs));
        }

        vector<Field> fields;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            const Field& field = type->field(i);
            const string identifier = declare_variable(field.name());
            fields.emplace_back(field.modifiers(), declared_type_alias(field.type()), identifier, /*is_multi_var_def*/true);
            assign_values(identifier, field_values[i], /*is_definition*/true);
        }

        result.push_back(statements::create_statement<MultiVariableDefinition>(create_type(std::move(fields)), std::move(expr)));
    }

    void GraphDecompiler::create_variable_assignment(const VariableAssignment& assignment, const Instance& instance, vector<StmtPtr>& result)
    {
        const ExprPtr& lhs = assignment.lhs_expression();
        const IdentifierPtr root = root_identifier(lhs);
        if (root == nullptr)
            return;

        VariableInfo* var = find_variable(root->name());
        const string identifier = var ? var->identifier : root->name();
        const ExprPtr renamed_lhs = rename_root(lhs, identifier);

        const vector<HintedGraph::BoundValue> values = hinted_->statement_values(instance, root->name());
        if (values.empty())
        {
            const DocumentHints::StatementRecord* record = hinted_->hints().statement(instance.frame.id);
            if (record)
                create_record_statement(*record, result);
            return;
        }

        ExprPtr rhs;
        ExprPtr target = renamed_lhs;
        if (values.size() == 1 and values.front().path.empty())
        {
            const DotOperatorPtr dot = cast_expression<DotOperator>(lhs);
            if (dot and cast_expression<Identifier>(dot->value_expression()))
            {
                // `q.y = x;` is compiled to a combine node of the channels of q and x
                rhs = create_swizzle_assignment(identifier, dot->token().lexeme(), values.front().value);
            }

            // an element of a vector, e.g., `v[1] = x;`
            const IndexingOperatorPtr indexing = cast_expression<IndexingOperator>(lhs);
            if (indexing and cast_expression<Identifier>(indexing->value_expression()))
            {
                if (optional<IndexAssignment> index = create_index_assignment(identifier, values.front().value))
                {
                    target = std::move(index->target);
                    rhs = std::move(index->value);
                }
            }

            if (rhs == nullptr)
            {
                target = create_identifier(identifier);
                rhs = create_value_expression(values.front().value);
            }
        }
        else if (values.size() == 1 and cast_expression<DotOperator>(lhs))
        {
            // a field of a struct, e.g., `ray.origin = p;`
            rhs = create_value_expression(values.front().value);
        }
        else
        {
            target = create_identifier(identifier);
            rhs = create_struct_expression(values, nullptr);
        }

        if (rhs == nullptr)
            return;

        result.push_back(statements::create_statement<VariableAssignment>(create_symbol("="), std::move(target), std::move(rhs)));
        assign_values(identifier, values, /*is_definition*/false);
        if (var)
            var->statement.reset();
    }

    optional<GraphDecompiler::IndexAssignment> GraphDecompiler::create_index_assignment(const string& identifier, const HintedGraph::Value& value)
    {
        const Instance* call = hinted_->hiding_call(value.node, level_);
        const DocumentHints::CallRecord* record = call ? hinted_->hints().call(call->frame.id) : nullptr;
        if (record == nullptr or record->function != "__set__" or record->arguments.size() != 3)
            return std::nullopt;

        // the value of the statement is the vector argument of the call, after the element was assigned to it
        const vector<string> vector_path{record->arguments.front().param};
        const vector<HintedGraph::BoundValue> call_values = hinted_->call_values(*call);
        const bool is_vector = std::any_of(call_values.begin(), call_values.end(), [&](const HintedGraph::BoundValue& call_value) {
            return call_value.value == value and call_value.path == vector_path;
        });
        if (not is_vector)
            return std::nullopt;

        // the vector, index and value arguments
        vector<ExprPtr> args;
        vector<mx::InputPtr> inputs;
        for (const DocumentHints::Argument& arg : record->arguments)
        {
            const vector<mx::InputPtr> arg_inputs = hinted_->argument_inputs(*call, arg.param);
            if (arg_inputs.empty())
                return std::nullopt;

            TypedContext context{is_typed_context_, true};
            ExprPtr expr = create_port_expression(arg_inputs.front());
            if (expr == nullptr)
                return std::nullopt;
            args.push_back(std::move(expr));
            inputs.push_back(arg_inputs.front());
        }

        // the element of another vector cannot be assigned to the variable, e.g., `v = u; v[1] = x;` where v is u
        if (args[0]->to_string() != identifier)
            return std::nullopt;

        return IndexAssignment{create_expression<IndexingOperator>(create_identifier(identifier), args[1]), args[2], inputs[2]};
    }

    void GraphDecompiler::create_port_assignment(const VariableAssignment& assignment, const mx::InputPtr& input, vector<StmtPtr>& result)
    {
        const IdentifierPtr root = root_identifier(assignment.lhs_expression());
        const VariableInfo* var = root ? find_variable(root->name()) : nullptr;
        ExprPtr lhs = var ? rename_root(assignment.lhs_expression(), var->identifier) : assignment.lhs_expression();

        if (ExprPtr expr = create_port_expression(input))
            result.push_back(statements::create_statement<VariableAssignment>(create_symbol("="), std::move(lhs), std::move(expr)));
    }

    ExprPtr GraphDecompiler::create_swizzle_assignment(const string& var_name, const string& swizzle, const HintedGraph::Value& value)
    {
        const mx::NodePtr& combine = value.node;
        const string& category = combine->getCategory();
        if (category != "combine2" and category != "combine3" and category != "combine4")
            return nullptr;

        // the previous value of the variable, which provides the channels that are not assigned
        string previous_key;
        for (const auto& [key, holders] : state_.value_names)
        {
            for (const Holder& holder : holders)
            {
                if (holder.first == var_name and holder.second.empty())
                    previous_key = key;
            }
        }
        if (previous_key.empty())
            return nullptr;

        const bool is_color = is_color_type(combine->getType());
        const string channels = is_color ? "rgba" : "xyzw";
        const size_t count = static_cast<size_t>(category.back() - '0');

        vector<mx::InputPtr> assigned(swizzle.size());
        for (size_t i = 0; i < count; ++i)
        {
            const mx::InputPtr input = combine->getInput("in" + std::to_string(i + 1));
            if (input == nullptr)
                return nullptr;

            const size_t swizzle_index = swizzle.find(channels[i]);
            if (swizzle_index != string::npos)
            {
                assigned[swizzle_index] = input;
                continue;
            }

            // the other channels are separated from the previous value
            const mx::NodePtr separate = input->getConnectedNode();
            if (separate == nullptr or separate->getCategory().rfind("separate", 0) != 0 or input->getOutputString() != string{"out"} + channels[i])
                return nullptr;
            const mx::InputPtr separate_input = separate->getInput("in");
            const mx::NodePtr previous = separate_input ? separate_input->getConnectedNode() : nullptr;
            if (previous == nullptr or value_key(previous, separate_input->getOutputString()) != previous_key)
                return nullptr;
        }

        if (std::any_of(assigned.begin(), assigned.end(), [](const mx::InputPtr& input) { return input == nullptr; }))
            return nullptr;

        if (assigned.size() == 1)
            return create_port_expression(assigned.front());

        // `q.xz = v;` separates v and combines its channels with the other channels of q
        mx::NodePtr source;
        for (size_t i = 0; i < assigned.size(); ++i)
        {
            const mx::NodePtr separate = assigned[i]->getConnectedNode();
            const optional<char> channel = swizzle_channel(assigned[i]->getOutputString());
            if (separate == nullptr or separate->getCategory().rfind("separate", 0) != 0 or (source and separate != source) or not channel or (*channel != "xyzw"[i] and *channel != "rgba"[i]))
            {
                source = nullptr;
                break;
            }
            source = separate;
        }
        if (source and source->getInput("in"))
            return create_port_expression(source->getInput("in"));

        vector<ExprPtr> args;
        for (const mx::InputPtr& input : assigned)
            args.push_back(create_port_expression(input));
        return create_expression<NamedConstructor>(type_alias("vector" + std::to_string(assigned.size())), ArgumentList{std::as_const(args)});
    }

    void GraphDecompiler::create_expression_statement(const ExpressionStatement& stmt, const Instance& instance, vector<StmtPtr>& result)
    {
        const ExprPtr& expr = stmt.expression();

        // `x += y;` and `x++;` are compiled to calls to __add__ and __inc__ whose results are assigned to x
        const CompoundAssignmentPtr compound = cast_expression<CompoundAssignment>(expr);
        const IncrementOperatorPtr increment = cast_expression<IncrementOperator>(expr);
        if (compound or increment)
        {
            const ExprPtr& lhs = compound ? compound->lhs_expression() : increment->value_expression();
            // the variable, or the vector of an element of a vector, e.g., `v[1] += x;`
            const IndexingOperatorPtr indexing = cast_expression<IndexingOperator>(lhs);
            const IdentifierPtr root = cast_expression<Identifier>(indexing ? indexing->value_expression() : lhs);
            if (root == nullptr)
                return;

            VariableInfo* var = find_variable(root->name());
            const string identifier = var ? var->identifier : root->name();
            const vector<HintedGraph::BoundValue> values = hinted_->statement_values(instance, root->name());
            if (values.size() != 1 or not values.front().path.empty())
            {
                const DocumentHints::StatementRecord* record = hinted_->hints().statement(instance.frame.id);
                if (record and values.empty())
                    create_record_statement(*record, result);
                return;
            }

            const HintedGraph::Value& value = values.front().value;
            const string dunder = compound ? compound_dunders().at(compound->token().lexeme()) : (increment->token() == TokenType::Increment ? "__inc__" : "__dec__");

            // what was assigned to, and the result of the operator, which is assigned to the element of `v[1] += x;`
            ExprPtr target = create_identifier(identifier);
            ExprPtr assigned_value;
            HintedGraph::Value result_value = value;
            if (indexing)
            {
                optional<IndexAssignment> index = create_index_assignment(identifier, value);
                if (index)
                {
                    target = std::move(index->target);
                    assigned_value = std::move(index->value);
                    result_value = HintedGraph::Value{connected_node(index->value_input), index->value_input->getOutputString()};
                }
            }

            StmtPtr created;
            const Instance* call = result_value.node ? hinted_->hiding_call(result_value.node, level_) : nullptr;
            const DocumentHints::CallRecord* record = call ? hinted_->hints().call(call->frame.id) : nullptr;
            if (record and record->function == dunder and not record->arguments.empty())
            {
                const vector<mx::InputPtr> lhs_inputs = hinted_->argument_inputs(*call, record->arguments.front().param);
                const ExprPtr lhs_value = lhs_inputs.empty() ? nullptr : create_port_expression(lhs_inputs.front());
                const bool is_same_target = lhs_value and lhs_value->to_string() == target->to_string();

                if (is_same_target and increment)
                {
                    created = statements::create_statement<ExpressionStatement>(
                        create_expression<IncrementOperator>(target, increment->token(), increment->is_prefix())
                    );
                }
                else if (is_same_target and record->arguments.size() == 2)
                {
                    const vector<mx::InputPtr> rhs_inputs = hinted_->argument_inputs(*call, record->arguments[1].param);
                    if (ExprPtr rhs = rhs_inputs.empty() ? nullptr : create_port_expression(rhs_inputs.front()))
                    {
                        created = statements::create_statement<ExpressionStatement>(
                            create_expression<CompoundAssignment>(target, compound->token(), std::move(rhs))
                        );
                    }
                }
            }

            // otherwise the assignment is created without the operator, e.g., `x = x + y;`
            if (created == nullptr)
            {
                ExprPtr rhs = assigned_value ? std::move(assigned_value) : create_value_expression(value);
                created = statements::create_statement<VariableAssignment>(create_symbol("="), std::move(target), std::move(rhs));
            }

            result.push_back(std::move(created));
            assign_values(identifier, values, /*is_definition*/false);
            if (var)
                var->statement.reset();
            return;
        }

        // the roots of the statement are expressions whose values are not used, e.g., `polar(uv, float r, float a);`
        for (const auto& [call, node] : statement_roots(instance))
        {
            if (call)
            {
                TypedContext context{is_typed_context_, false};
                if (ExprPtr call_expr = create_call_expression(*call))
                    result.push_back(statements::create_statement<ExpressionStatement>(std::move(call_expr)));
            }
            else if (not out_parameter_outputs(node).empty())
            {
                // out arguments of node calls are declared by the call, e.g., `sincos(x, float s, float c);`
                declare_out_arguments(node);
                TypedContext context{is_typed_context_, false};
                result.push_back(statements::create_statement<ExpressionStatement>(create_function_call(node, /*with_out_arguments*/true)));
            }
            else
            {
                TypedContext context{is_typed_context_, false};
                result.push_back(statements::create_statement<ExpressionStatement>(create_node_expression(node)));
            }
        }
    }

    void GraphDecompiler::create_loop(const ForEachLoop& loop, const Instance& instance, vector<StmtPtr>& result)
    {
        // the values that were iterated over, e.g., {1, 2, 3}
        vector<string> values;
        const auto iter_values = cast_expression<UnnamedConstructor>(loop.iter_expression());
        if (iter_values)
        {
            for (const ExprPtr& value : iter_values->expressions())
                values.push_back(value->to_string());
        }

        vector<const Instance*> iterations;
        for (const Instance* child : instance.children)
        {
            if (child->is_loop_iteration())
                iterations.push_back(child);
        }
        std::sort(iterations.begin(), iterations.end(), [](const Instance* a, const Instance* b) { return a->frame.id < b->frame.id; });

        const vector<size_t> body_ids = hinted_->hints().children(instance.frame.id);

        bool is_loop = not iterations.empty() and iterations.size() == values.size() and not contains(values, hints::PLACEHOLDER);
        for (size_t i = 0; is_loop and i < iterations.size(); ++i)
            is_loop = iterations[i]->frame.id == i;

        const HintState saved_state = state_;
        vector<StmtPtr> body;
        string first_body;
        for (size_t i = 0; is_loop and i < iterations.size(); ++i)
        {
            push_scope();
            is_loop = declare_variable(loop.name(), instance.frame.id) == loop.name();

            vector<StmtPtr> iteration_body;
            create_region(body_ids, *iterations[i], iteration_body);

            // variables declared in the body cannot be used after the loop
            for (const HintedGraph::Value& value : pop_scope())
            {
                for (const Use& use : uses_.count(value.node) ? uses_.at(value.node) : vector<Use>{})
                {
                    const mx::NodePtr consumer = use.consumer->asA<mx::Node>();
                    const Instance* consumer_instance = consumer ? hinted_->instance(consumer) : nullptr;
                    if (consumer_instance == nullptr or not hinted_->is_within(*consumer_instance, instance))
                        is_loop = false;
                }
            }

            const string text = join_statements(iteration_body);
            if (i == 0)
            {
                first_body = text;
                body = std::move(iteration_body);
            }
            else if (text != first_body)
            {
                is_loop = false;
            }
        }

        if (is_loop)
        {
            // consecutive integers are written as a range, e.g., `0 to 3`
            ExprPtr iter_expr = loop.iter_expression();
            bool is_range = values.size() > 1;
            for (size_t i = 0; is_range and i < values.size(); ++i)
                is_range = is_index(values[i]) and std::stoll(values[i]) == std::stoll(values.front()) + static_cast<long long>(i);
            if (is_range)
                iter_expr = create_expression<RangeExpression>(iter_values->expressions().front(), iter_values->expressions().back(), Token{TokenType::To, "to"});

            result.push_back(loop.with_body(std::move(iter_expr), statements::create_statement<BlockStatement>(std::move(body))));
            return;
        }

        // the iterations create different code, so the loop is unrolled
        state_ = saved_state;
        for (const Instance* iteration : iterations)
            create_region(body_ids, *iteration, result);
    }

    void GraphDecompiler::create_return(const DocumentHints::StatementRecord& statement, vector<StmtPtr>& result)
    {
        // the return value of the call, which excludes its out parameters
        vector<HintedGraph::BoundValue> values;
        if (body_call_)
        {
            const DocumentHints::CallRecord* record = hinted_->hints().call(body_call_->frame.id);
            for (const HintedGraph::BoundValue& value : hinted_->call_values(*body_call_))
            {
                const bool is_out_param = record and not value.path.empty() and std::any_of(record->arguments.begin(), record->arguments.end(), [&](const DocumentHints::Argument& arg) {
                    return arg.param == value.path.front();
                });
                if (not is_out_param)
                    values.push_back(value);
            }
        }

        // compile-time values are created from the skeleton, e.g., `return x;`
        if (values.empty())
        {
            create_record_statement(statement, result);
            return;
        }

        const auto return_stmt = dynamic_cast<const ReturnStatement*>(statement.statement.get());
        const ExprPtr skeleton = return_stmt ? return_stmt->expression() : nullptr;
        ExprPtr expr = values.size() == 1 and values.front().path.empty() ? create_value_expression(values.front().value) : create_struct_expression(values, nullptr, skeleton);
        if (expr)
            result.push_back(statements::create_statement<ReturnStatement>(std::move(expr)));
    }

    ExprPtr GraphDecompiler::create_hinted_output_expression(const mx::NodePtr& node, const string& output_name)
    {
        // values held by variables are referenced by the variable
        const string key = value_key(node, output_name);
        const auto name = state_.value_names.find(key);
        if (name != state_.value_names.end())
            return variable_expression(name->second.back().first, name->second.back().second);
        if (const auto snapshot = state_.snapshot_names.find(key); snapshot != state_.snapshot_names.end())
            return create_identifier(snapshot->second);

        // a value of another statement that is no longer held by its variable
        const vector<DocumentHints::Binding> bindings = DocumentHints::bindings(node);
        const bool is_variable = std::any_of(bindings.begin(), bindings.end(), [](const DocumentHints::Binding& binding) { return binding.frame.kind == 's'; });
        if (is_variable and not (level_ and hinted_->is_within(*hinted_->instance(node), *level_)))
            stale_values_.insert(key);

        // the nodes of collapsed calls are hidden, only their return value is visible
        if (const Instance* call = hinted_->hiding_call(node, level_))
        {
            for (const HintedGraph::BoundValue& value : hinted_->call_values(*call))
            {
                if (value.value.node != node or not is_same_output(value.value.output, output_name))
                    continue;
                if (value.path.empty())
                    return create_call_expression(*call);

                // a field of a struct return value, out parameters are held by variables
                const DocumentHints::CallRecord* record = hinted_->hints().call(call->frame.id);
                const bool is_out_param = record and std::any_of(record->arguments.begin(), record->arguments.end(), [&](const DocumentHints::Argument& arg) {
                    return arg.param == value.path.front();
                });
                if (is_out_param)
                    continue;

                ExprPtr call_expr = create_call_expression(*call);
                if (call_expr == nullptr)
                    return nullptr;
                const string& field = value.path.front();
                if (is_index(field))
                    return create_expression<IndexingOperator>(std::move(call_expr), create_expression<Literal>(Primitive{std::stoi(field)}));
                return create_expression<DotOperator>(std::move(call_expr), Token{TokenType::Identifier, field});
            }
        }

        // an output of a multi-output node that is not held by a variable, e.g., `separate2(v).outx`
        const string resolved_output = resolved_output_name(node, output_name);
        if (node->getType() == mx::MULTI_OUTPUT_TYPE_STRING and not resolved_output.empty() and not is_statement(node))
        {
            // nonlocal variables that the call assigned to
            if (serialize::has_prefix(resolved_output, serialize::NONLOCAL_OUT_PREFIX))
                return create_identifier(serialize::remove_prefix(resolved_output));
            if (serialize::has_prefix(resolved_output, serialize::OUT_PARAMETER_PREFIX))
                return nullptr;

            // the only return value is the value of the call, e.g., of a function that also assigns a nonlocal variable
            const vector<mx::OutputPtr> outputs = return_outputs(node);
            if (outputs.size() == 1 and outputs.front()->getName() == resolved_output)
                return create_node_expression(node);

            const string field = serialize::has_prefix(resolved_output, serialize::RETURN_VALUE_PREFIX) ? serialize::remove_prefix(resolved_output) : resolved_output;
            if (ExprPtr call = create_node_expression(node))
                return create_expression<DotOperator>(std::move(call), Token{TokenType::Identifier, to_identifier(field)});
        }

        return nullptr;
    }

    ExprPtr GraphDecompiler::create_hinted_input_expression(const mx::InputPtr& input, const size_t first_source)
    {
        const vector<DocumentHints::Source> sources = DocumentHints::sources(input);
        for (size_t i = first_source; i < sources.size(); ++i)
        {
            const DocumentHints::Source& source = sources[i];

            // the compile-time return value of a call of the statement, e.g., `Drops(uv, t).x`
            if (source.frame.kind == 'c' and source.name.empty())
            {
                const Instance* call = hinted_->call(source.frame.id);
                if (call and hinted_->is_collapsed(*call) and level_ and hinted_->is_within(*call, *level_) and DocumentHints::is_unedited(input))
                    return create_call_expression(*call);
                break;
            }

            // the parameters of the function whose body is being created
            if (source.frame.kind == 'c')
            {
                if (body_call_ and source.frame == body_call_->frame)
                    return create_identifier(source.name);
                continue;
            }

            if (source.frame.kind == 's')
            {
                // a variable, or a field of a struct variable, e.g., "xyz.1"
                vector<string> path;
                for (size_t start = 0, end = 0; end != string::npos; start = end + 1)
                {
                    end = source.name.find('.', start);
                    path.push_back(source.name.substr(start, end == string::npos ? string::npos : end - start));
                }
                const VariableInfo* var = find_variable(path.front());
                path.erase(path.begin());
                if (var == nullptr)
                    break;

                // fields that hold node outputs are referenced if they still hold them, e.g., `xyz[0]`, and copies of other
                // variables if they were not assigned to since, e.g., uv of `vec2 uv = fragCoord;`
                if (const mx::NodePtr node = connected_node(input))
                {
                    const bool is_field = not path.empty() and is_held_by(HintedGraph::Value{node, input->getOutputString()}, Holder{var->identifier, path});
                    const bool is_copy = path.empty() and var->statement == source.frame.id;
                    if (is_field or is_copy)
                        return variable_expression(var->identifier, path);
                    break;
                }

                // compile-time values are only referenced if the variable still holds them, i.e., if the variable was not
                // assigned to since and the value of the input was not edited
                if (var->statement == source.frame.id and DocumentHints::is_unedited(input))
                    return variable_expression(var->identifier, path);
                break;
            }
        }
        return nullptr;
    }

    ExprPtr GraphDecompiler::create_call_expression(const Instance& call)
    {
        const DocumentHints::CallRecord* record = hinted_->hints().call(call.frame.id);
        if (record == nullptr)
            return nullptr;

        const vector<HintedGraph::BoundValue> values = hinted_->call_values(call);

        vector<ExprPtr> exprs;
        vector<Argument> args;
        for (const DocumentHints::Argument& arg : record->arguments)
        {
            ExprPtr expr;
            const vector<mx::InputPtr> inputs = hinted_->argument_inputs(call, arg.param);
            if (not inputs.empty())
            {
                TypedContext context{is_typed_context_, true};
                expr = create_port_expression(inputs.front());
            }
            else
            {
                // the fields of struct out parameters are assigned to the fields of a variable, e.g., `foo(xyz)`
                expr = create_struct_out_argument(values, arg.param);

                // out parameters are assigned to a variable, which can be declared by the call
                for (const HintedGraph::BoundValue& value : values)
                {
                    if (expr or value.path.size() != 1 or value.path.front() != arg.param)
                        continue;

                    for (const DocumentHints::Binding& binding : DocumentHints::bindings(value.value.node))
                    {
                        if (binding.frame.kind != 's' or level_ == nullptr or not (binding.frame == level_->frame) or not is_same_output(binding.output, value.value.output))
                            continue;

                        const string& var_name = binding.path.front();
                        if (const VariableInfo* var = find_variable(var_name))
                        {
                            expr = create_identifier(var->identifier);
                            assign_values(var->identifier, {HintedGraph::BoundValue{value.value, {}}}, /*is_definition*/false);
                        }
                        else
                        {
                            const string identifier = declare_variable(var_name);
                            const string output_type = value.value.output.empty() ? value.value.node->getType() : value.value.node->getOutput(value.value.output) ? value.value.node->getOutput(value.value.output)->getType() : value.value.node->getType();
                            expr = create_expression<VariableDefinitionExpression>(ModifierList{}, create_type_from(output_type), Token{TokenType::Identifier, identifier});
                            assign_values(identifier, {HintedGraph::BoundValue{value.value, {}}}, /*is_definition*/true);
                        }
                        break;
                    }
                }
            }

            // the code of out arguments and compile-time values that are not part of the graph, e.g., `out fragColor`
            ModifierList mods;
            if (expr == nullptr and arg.code)
            {
                optional<Argument> code_arg = parse_argument(*arg.code);
                if (code_arg)
                {
                    mods = code_arg->modifiers();
                    expr = rename_variables(code_arg->expression());

                    // the call assigned the out argument, e.g., its default value if the function did not assign it
                    const IdentifierPtr root = mods.contains(TokenType::Out) ? root_identifier(expr) : nullptr;
                    if (VariableInfo* var = root ? find_variable_by_identifier(root->name()) : nullptr; var and level_ and level_->is_statement())
                        var->statement = level_->frame.id;
                }
            }

            if (expr == nullptr)
                return nullptr;

            exprs.push_back(expr);
            args.emplace_back(AttributeList{}, std::move(mods), arg.is_named ? arg.param : "", std::move(expr), args.size());
        }

        const string& function = record->function;
        const bool has_named_argument = std::any_of(record->arguments.begin(), record->arguments.end(), [](const DocumentHints::Argument& arg) { return arg.is_named; });

        if (not has_named_argument)
        {
            if (contains(binary_dunders(), function) and exprs.size() == 2)
                return ExpressionFactory::binary(exprs[0], create_symbol(binary_dunders().at(function)), exprs[1]);
            if (contains(unary_dunders(), function) and exprs.size() == 1)
                return ExpressionFactory::unary(create_symbol(unary_dunders().at(function)), exprs[0]);
            if (function == "__abs__" and exprs.size() == 1)
                return ExpressionFactory::absolute(exprs[0], create_symbol("|"));
            if (function == "__get__" and exprs.size() == 2)
                return create_expression<IndexingOperator>(exprs[0], exprs[1]);
            if ((function == "__inc__" or function == "__dec__") and exprs.size() == 1)
            {
                if (ExprPtr increment = create_increment_expression(call, function == "__inc__", exprs[0]))
                    return increment;
            }
            if (const optional<string> type = constructor_type(function))
                return create_expression<NamedConstructor>(*type, ArgumentList{std::move(args)});
        }

        return create_expression<FunctionCall>(function, nullptr, ArgumentList{std::move(args)});
    }

    ExprPtr GraphDecompiler::create_struct_out_argument(const vector<HintedGraph::BoundValue>& call_values, const string& param_name)
    {
        // the fields of the parameter that the call assigned, e.g., p.x and p.z, which are fields of a variable of the statement
        VariableInfo* var = nullptr;
        vector<HintedGraph::BoundValue> values;
        for (const HintedGraph::BoundValue& value : call_values)
        {
            if (value.path.size() < 2 or value.path.front() != param_name)
                continue;

            for (const DocumentHints::Binding& binding : DocumentHints::bindings(value.value.node))
            {
                if (binding.frame.kind != 's' or level_ == nullptr or not (binding.frame == level_->frame) or binding.path.size() < 2 or not is_same_output(binding.output, value.value.output))
                    continue;

                VariableInfo* binding_var = find_variable(binding.path.front());
                if (binding_var == nullptr or (var and binding_var != var))
                    return nullptr;
                var = binding_var;

                const HintedGraph::BoundValue field{value.value, {binding.path.begin() + 1, binding.path.end()}};
                const bool is_new = std::none_of(values.begin(), values.end(), [&](const HintedGraph::BoundValue& other) { return other.path == field.path; });
                if (is_new)
                    values.push_back(field);
            }
        }

        if (var == nullptr or values.empty())
            return nullptr;

        assign_values(var->identifier, values, /*is_definition*/false);
        // the fields that were not assigned keep their compile-time values, which are now referenced by the statement
        var->statement = level_->frame.id;
        return create_identifier(var->identifier);
    }

    ExprPtr GraphDecompiler::create_increment_expression(const Instance& call, const bool is_increment, const ExprPtr& arg)
    {
        // `++x` assigns the value of the call to x, which is referenced by x afterwards, e.g., `foo(++x, ++x)`
        const IdentifierPtr identifier = cast_expression<Identifier>(arg);
        VariableInfo* var = identifier ? find_variable_by_identifier(identifier->name()) : nullptr;
        if (var == nullptr)
            return nullptr;

        const vector<HintedGraph::BoundValue> values = hinted_->call_values(call);
        if (values.size() != 1 or not values.front().path.empty())
            return nullptr;

        assign_values(var->identifier, {HintedGraph::BoundValue{values.front().value, {}}}, /*is_definition*/false);
        var->statement.reset();

        const Token op = is_increment ? Token{TokenType::Increment, "++"} : Token{TokenType::Decrement, "--"};
        return create_expression<IncrementOperator>(arg, op, /*prefix*/true);
    }

    ExprPtr GraphDecompiler::create_value_expression(const HintedGraph::Value& value)
    {
        return create_output_expression(value.node, value.output);
    }

    ExprPtr GraphDecompiler::create_struct_expression(const vector<HintedGraph::BoundValue>& values, const TypePtr& type, const ExprPtr& skeleton)
    {
        if (ExprPtr expr = create_multi_output_expression(values))
            return expr;

        // the field names in the order of the type, or in the order the values were created if the type is unknown
        vector<string> field_names;
        if (type and type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
                field_names.push_back(type->field(i).has_name() ? type->field_name(i) : std::to_string(i));
        }
        else
        {
            for (const HintedGraph::BoundValue& value : values)
            {
                if (not value.path.empty() and not contains(field_names, value.path.front()))
                    field_names.push_back(value.path.front());
            }
            if (std::all_of(field_names.begin(), field_names.end(), is_index))
            {
                // the fields without nodes are part of the skeleton, e.g., `{_, 1.0}`
                const auto skeleton_fields = cast_expression<UnnamedConstructor>(skeleton);
                if (skeleton_fields)
                {
                    field_names.clear();
                    for (size_t i = 0; i < skeleton_fields->expressions().size(); ++i)
                        field_names.push_back(std::to_string(i));
                }
                std::sort(field_names.begin(), field_names.end(), [](const string& a, const string& b) { return std::stoi(a) < std::stoi(b); });
            }
        }

        vector<ExprPtr> fields;
        for (size_t i = 0; i < field_names.size(); ++i)
        {
            vector<HintedGraph::BoundValue> field_values;
            for (const HintedGraph::BoundValue& value : values)
            {
                if (not value.path.empty() and (value.path.front() == field_names[i] or value.path.front() == std::to_string(i)))
                    field_values.push_back(HintedGraph::BoundValue{value.value, {value.path.begin() + 1, value.path.end()}});
            }

            // fields without nodes are compile-time values, which are part of the skeleton, e.g., `{_, 1.0}`
            const auto skeleton_fields = cast_expression<UnnamedConstructor>(skeleton);
            const ExprPtr skeleton_field = skeleton_fields and i < skeleton_fields->expressions().size() ? skeleton_fields->expressions()[i] : nullptr;

            if (field_values.size() == 1 and field_values.front().path.empty())
                fields.push_back(create_value_expression(field_values.front().value));
            else if (not field_values.empty())
                fields.push_back(create_struct_expression(field_values, type ? resolve_type(type->field_type(i)) : nullptr, skeleton_field));
            else if (skeleton_field and not is_placeholder(skeleton_field))
                fields.push_back(skeleton_field);
            else
                return nullptr;

            if (fields.back() == nullptr)
                return nullptr;
        }

        return create_expression<UnnamedConstructor>(std::move(fields));
    }

    vector<std::pair<const HintedGraph::Instance*, mx::NodePtr>> GraphDecompiler::statement_roots(const Instance& instance) const
    {
        // the nodes and collapsed calls of the statement, dissolved calls are part of the statement itself
        vector<std::pair<const Instance*, mx::NodePtr>> candidates;
        std::function<void(const Instance&)> collect = [&](const Instance& frame) {
            for (const mx::NodePtr& node : frame.nodes)
                candidates.emplace_back(nullptr, node);
            for (const Instance* child : frame.children)
            {
                if (hinted_->is_collapsed(*child))
                    candidates.emplace_back(child, nullptr);
                else
                    collect(*child);
            }
        };
        collect(instance);

        const auto is_used_by_statement = [&](const mx::NodePtr& node, const Instance* excluded) {
            if (not contains(uses_, node))
                return false;
            for (const Use& use : uses_.at(node))
            {
                const mx::NodePtr consumer = use.consumer->asA<mx::Node>();
                const Instance* consumer_instance = consumer ? hinted_->instance(consumer) : nullptr;
                if (consumer_instance and hinted_->is_within(*consumer_instance, instance) and not (excluded and hinted_->is_within(*consumer_instance, *excluded)))
                    return true;
            }
            return false;
        };

        vector<std::pair<const Instance*, mx::NodePtr>> result;
        for (const auto& [call, node] : candidates)
        {
            bool is_used = false;
            if (call)
            {
                for (const mx::NodePtr& call_node : hinted_->nodes(*call))
                    is_used = is_used or is_used_by_statement(call_node, call);
            }
            else
            {
                // nodes absorbed by the pattern of another node are part of that node's expression
                is_used = is_used_by_statement(node, nullptr) or contains(absorbed_, node);
            }

            if (not is_used)
                result.emplace_back(call, node);
        }
        return result;
    }

    GraphDecompiler::VariableInfo* GraphDecompiler::find_variable_by_identifier(const string& identifier)
    {
        for (auto it = state_.scopes.rbegin(); it != state_.scopes.rend(); ++it)
        {
            for (auto& [name, var] : it->variables)
            {
                if (var.identifier == identifier)
                    return &var;
            }
        }
        return nullptr;
    }

    GraphDecompiler::VariableInfo* GraphDecompiler::find_variable(const string& name)
    {
        for (auto it = state_.scopes.rbegin(); it != state_.scopes.rend(); ++it)
        {
            const auto var = it->variables.find(name);
            if (var != it->variables.end())
                return &var->second;
        }
        return nullptr;
    }

    string GraphDecompiler::declare_variable(const string& name, const optional<size_t> statement, const string& preferred_identifier)
    {
        const auto is_used = [&](const string& identifier) {
            // the variables of a function body can have the same names as the variables of the graph
            if (not state_.is_function_body and contains(used_identifiers_, identifier))
                return true;
            return std::any_of(state_.scopes.begin(), state_.scopes.end(), [&](const HintScope& scope) {
                return contains(scope.identifiers, identifier);
            });
        };

        const string base = preferred_identifier.empty() ? to_identifier(name) : preferred_identifier;
        // e.g., x2, or x_2 if the name already ends with a digit
        const string separator = std::isdigit(static_cast<unsigned char>(base.back())) ? "_" : "";
        string identifier = base;
        for (size_t i = 2; is_used(identifier); ++i)
            identifier = base + separator + std::to_string(i);

        HintScope& scope = state_.scopes.back();
        scope.variables[name] = VariableInfo{identifier, statement};
        scope.identifiers.insert(identifier);
        return identifier;
    }

    ExprPtr GraphDecompiler::rename_variables(const ExprPtr& expr)
    {
        if (expr == nullptr)
            return nullptr;

        vector<Token> tokens;
        try
        {
            tokens = DocumentHints::scan_skeleton(expr->to_string());
        }
        catch (...)
        {
            return expr;
        }

        bool is_renamed = false;
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            if (tokens[i] != TokenType::Identifier)
                continue;

            // fields, e.g., `v.x`, functions, e.g., `foo(x)`, and named arguments, e.g., `foo(v = x)`, are not variables
            const bool is_field = i > 0 and tokens[i - 1] == '.';
            const bool is_call = i + 1 < tokens.size() and tokens[i + 1] == '(';
            const bool is_named_argument = i + 1 < tokens.size() and tokens[i + 1] == '=';
            if (is_field or is_call or is_named_argument)
                continue;

            const VariableInfo* var = find_variable(tokens[i].lexeme());
            if (var and var->identifier != tokens[i].lexeme())
            {
                tokens[i] = Token{TokenType::Identifier, var->identifier};
                is_renamed = true;
            }
        }

        if (not is_renamed)
            return expr;

        try
        {
            Parser parser{std::move(tokens)};
            return parser.expression();
        }
        catch (...)
        {
            return expr;
        }
    }

    void GraphDecompiler::push_scope()
    {
        state_.scopes.emplace_back();
    }

    vector<HintedGraph::Value> GraphDecompiler::pop_scope()
    {
        const unordered_set<string> identifiers = state_.scopes.back().identifiers;
        state_.scopes.pop_back();

        // the values that are no longer held by any variable
        vector<HintedGraph::Value> result;
        for (auto it = state_.value_names.begin(); it != state_.value_names.end();)
        {
            vector<Holder>& holders = it->second;
            holders.erase(std::remove_if(holders.begin(), holders.end(), [&](const Holder& holder) { return contains(identifiers, holder.first); }), holders.end());
            if (holders.empty())
            {
                const string& key = it->first;
                const size_t dot = key.rfind('.');
                if (const mx::NodePtr node = graph_->getNode(key.substr(0, dot)))
                    result.push_back(HintedGraph::Value{node, key.substr(dot + 1)});
                it = state_.value_names.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return result;
    }

    void GraphDecompiler::assign_values(const string& identifier, const vector<HintedGraph::BoundValue>& values, const bool is_definition)
    {
        // the variable no longer holds its previous values, or only the fields that were assigned to
        const auto is_replaced = [&](const Holder& holder) {
            if (holder.first != identifier)
                return false;
            if (is_definition or values.empty())
                return true;
            return std::any_of(values.begin(), values.end(), [&](const HintedGraph::BoundValue& value) {
                const vector<string>& path = holder.second;
                return path.size() >= value.path.size() and std::equal(value.path.begin(), value.path.end(), path.begin());
            });
        };
        for (auto it = state_.value_names.begin(); it != state_.value_names.end();)
        {
            vector<Holder>& holders = it->second;
            holders.erase(std::remove_if(holders.begin(), holders.end(), is_replaced), holders.end());
            it = holders.empty() ? state_.value_names.erase(it) : std::next(it);
        }

        for (const HintedGraph::BoundValue& value : values)
        {
            const string key = value_key(value.value.node, value.value.output);
            state_.value_names[key].emplace_back(identifier, value.path);
            if (contains(snapshot_values_, key))
                pending_snapshots_.emplace_back(key, value.value);
        }
    }

    bool GraphDecompiler::is_held_by(const HintedGraph::Value& value, const Holder& holder) const
    {
        const auto holders = state_.value_names.find(value_key(value.node, value.output));
        return holders != state_.value_names.end() and contains(holders->second, holder);
    }

    ExprPtr GraphDecompiler::variable_expression(const string& identifier, const vector<string>& path) const
    {
        ExprPtr expr = create_identifier(identifier);
        for (const string& field : path)
        {
            if (is_index(field))
                expr = create_expression<IndexingOperator>(std::move(expr), create_expression<Literal>(Primitive{std::stoi(field)}));
            else
                expr = create_expression<DotOperator>(std::move(expr), Token{TokenType::Identifier, field});
        }
        return expr;
    }

    TypePtr GraphDecompiler::resolve_type(const TypePtr& type) const
    {
        if (type == nullptr or type->has_fields())
            return type;

        // aliases declared by using declarations, e.g., `using Ray = {vec3 origin, vec3 dir};`
        const string name = type->to_string();
        for (const size_t id : hinted_->hints().children(0))
        {
            const DocumentHints::StatementRecord* record = hinted_->hints().statement(id);
            const auto using_decl = record ? dynamic_cast<const UsingDeclaration*>(record->statement.get()) : nullptr;
            if (using_decl and using_decl->name() == name)
                return resolve_type(using_decl->type());
        }
        return type;
    }
}
