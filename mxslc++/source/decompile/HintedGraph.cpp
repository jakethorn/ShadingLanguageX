//
// Created by jaket on 28/09/2026.
//

#include "decompile/HintedGraph.h"

#include <functional>

#include "runtime/Field.h"
#include "runtime/Type.h"
#include "statements/MultiVariableDefinition.h"
#include "statements/ReturnStatement.h"
#include "statements/VariableAssignment.h"
#include "statements/VariableDefinition.h"
#include "utils/container_utils.h"

namespace mxslc::decompile
{
    using container_utils::contains;

    namespace
    {
        string frame_key(const DocumentHints::Frame& frame)
        {
            return string{frame.kind} + std::to_string(frame.id);
        }

        // the value of an input, used to check that all inputs of an argument received the same value
        string input_value_key(const mx::InputPtr& input)
        {
            if (not input->getNodeName().empty())
                return "node:" + input->getNodeName() + "." + input->getOutputString();
            if (not input->getNodeGraphString().empty())
                return "nodegraph:" + input->getNodeGraphString() + "." + input->getOutputString();
            if (not input->getInterfaceName().empty())
                return "interface:" + input->getInterfaceName();
            const mx::ValuePtr value = input->getValue();
            return "value:" + (value ? value->getValueString() : input->getValueString());
        }
    }

    HintedGraph::HintedGraph(const DocumentHints& hints, const mx::GraphElementPtr& graph)
        : hints_{hints}, graph_{graph}, nodes_{graph->getNodes()}
    {
        root_.frame = Frame{'r', 0};

        if (hints_.empty())
            return;

        for (const mx::NodePtr& node : nodes_)
        {
            for (const mx::InputPtr& input : node->getInputs())
            {
                if (const mx::NodePtr source = input->getConnectedNode())
                    uses_[source].push_back(Use{node, input->getOutputString(), input});
            }

            const vector<Frame> context = DocumentHints::context(node);
            if (context.empty())
                continue;

            Instance* instance = get_or_create(context, context.size());
            instance->nodes.push_back(node);
            node_instances_[node] = instance;
        }

        for (const mx::OutputPtr& output : graph_->getOutputs())
        {
            if (const mx::NodePtr source = graph_->getNode(output->getNodeName()))
                uses_[source].push_back(Use{output, output->getOutputString(), nullptr});
        }

        validate_statements(root_);
        validate_calls(root_);
    }

    const HintedGraph::Instance* HintedGraph::instance(const mx::NodePtr& node) const
    {
        const auto it = node_instances_.find(node);
        return it != node_instances_.end() ? it->second : nullptr;
    }

    const HintedGraph::Instance* HintedGraph::statement(const Instance& parent, const size_t id) const
    {
        for (const Instance* child : parent.children)
        {
            if (child->is_statement() and child->frame.id == id)
                return child;
        }
        return nullptr;
    }

    void HintedGraph::remove_statement(const mx::NodePtr& node)
    {
        const auto it = node_instances_.find(node);
        if (it == node_instances_.end())
            return;

        Instance* instance = it->second;
        while (instance->parent != &root_)
            instance = instance->parent;
        remove(*instance);
    }

    bool HintedGraph::is_collapsed(const Instance& call) const
    {
        return call.is_call() and not contains(dissolved_calls_, &call);
    }

    void HintedGraph::dissolve(const Instance& call)
    {
        dissolved_calls_.insert(&call);
    }

    void HintedGraph::dissolve_calls(const bool is_library_only)
    {
        for (const auto& [key, instance] : instances_)
        {
            if (not instance->is_call())
                continue;
            const DocumentHints::CallRecord* record = hints_.call(instance->frame.id);
            if (not is_library_only or record == nullptr or record->definition == 0)
                dissolve(*instance);
        }
    }

    bool HintedGraph::is_within(const Instance& inner, const Instance& outer) const
    {
        for (const Instance* instance = &inner; instance; instance = instance->parent)
        {
            if (instance == &outer)
                return true;
        }
        return false;
    }

    const HintedGraph::Instance* HintedGraph::call(const size_t id) const
    {
        for (const auto& [key, instance] : instances_)
        {
            if (instance->is_call() and instance->frame.id == id and is_attached(*instance))
                return instance.get();
        }
        return nullptr;
    }

    bool HintedGraph::is_attached(const Instance& instance) const
    {
        // removed statements are detached from their parent
        const Instance* current = &instance;
        for (; current->parent; current = current->parent)
        {
            const vector<Instance*>& siblings = current->parent->children;
            if (std::find(siblings.begin(), siblings.end(), current) == siblings.end())
                return false;
        }
        return current == &root_;
    }

    const HintedGraph::Instance* HintedGraph::hiding_call(const mx::NodePtr& node, const Instance* frame) const
    {
        const Instance* result = nullptr;
        for (const Instance* instance = this->instance(node); instance and instance != frame; instance = instance->parent)
        {
            if (is_collapsed(*instance))
                result = instance;
        }
        return result;
    }

    vector<HintedGraph::BoundValue> HintedGraph::statement_values(const Instance& statement, const string& var_name) const
    {
        vector<BoundValue> result;
        for (const mx::NodePtr& node : nodes(statement))
        {
            for (const Binding& binding : DocumentHints::bindings(node))
            {
                if (binding.frame == statement.frame and binding.path.front() == var_name)
                    result.push_back(BoundValue{Value{node, binding.output}, {binding.path.begin() + 1, binding.path.end()}});
            }
        }
        return result;
    }

    vector<HintedGraph::BoundValue> HintedGraph::call_values(const Instance& call) const
    {
        vector<BoundValue> result;
        for (const mx::NodePtr& node : nodes(call))
        {
            for (const Binding& binding : DocumentHints::bindings(node))
            {
                if (binding.frame == call.frame)
                    result.push_back(BoundValue{Value{node, binding.output}, binding.path});
            }
        }
        return result;
    }

    vector<HintedGraph::BoundValue> HintedGraph::alias_values(const size_t statement_id, const Instance& parent, const string& var_name) const
    {
        vector<BoundValue> result;
        for (const mx::NodePtr& node : nodes_)
        {
            const Instance* node_instance = instance(node);
            if (node_instance == nullptr or not (&parent == &root_ or is_within(*node_instance, parent)))
                continue;

            for (const Binding& binding : DocumentHints::bindings(node))
            {
                if (binding.frame.kind == 's' and binding.frame.id == statement_id and binding.path.front() == var_name)
                    result.push_back(BoundValue{Value{node, binding.output}, {binding.path.begin() + 1, binding.path.end()}});
            }
        }
        return result;
    }

    vector<mx::InputPtr> HintedGraph::argument_inputs(const Instance& call, const string& param_name) const
    {
        vector<mx::InputPtr> result;
        for (const mx::NodePtr& node : nodes(call))
        {
            for (const mx::InputPtr& input : node->getInputs())
            {
                for (const DocumentHints::Source& source : DocumentHints::sources(input))
                {
                    if (source.frame == call.frame and source.name == param_name)
                    {
                        result.push_back(input);
                        break;
                    }
                }
            }
        }
        return result;
    }

    vector<mx::NodePtr> HintedGraph::nodes(const Instance& frame) const
    {
        vector<mx::NodePtr> result = frame.nodes;
        for (const Instance* child : frame.children)
        {
            const vector<mx::NodePtr> child_nodes = nodes(*child);
            result.insert(result.end(), child_nodes.begin(), child_nodes.end());
        }
        return result;
    }

    HintedGraph::Instance* HintedGraph::get_or_create(const vector<Frame>& context, const size_t length)
    {
        string key;
        for (size_t i = 0; i < length; ++i)
            key += (i > 0 ? " " : "") + frame_key(context[i]);

        if (const auto it = instances_.find(key); it != instances_.end())
            return it->second.get();

        Instance* parent = length > 1 ? get_or_create(context, length - 1) : &root_;

        auto instance = std::make_unique<Instance>();
        instance->key = key;
        instance->frame = context[length - 1];
        instance->parent = parent;
        parent->children.push_back(instance.get());

        Instance* result = instance.get();
        instances_[key] = std::move(instance);
        return result;
    }

    void HintedGraph::validate_statements(Instance& parent)
    {
        for (Instance* child : vector<Instance*>{parent.children})
        {
            // the code of the graph and of loops is made of statements, other frames are created by code that is not
            // recorded, e.g., `print f();`
            const bool is_code = &parent == &root_ or parent.is_loop_iteration();
            if ((child->is_statement() and not is_valid_statement(*child)) or (is_code and not child->is_statement()))
                remove(*child);
            else
                validate_statements(*child);
        }
    }

    bool HintedGraph::is_valid_statement(const Instance& statement) const
    {
        const DocumentHints::StatementRecord* record = hints_.statement(statement.frame.id);
        if (record == nullptr or record->statement == nullptr)
            return false;

        // complete skeletons are the code of compile-time values, which only create the nodes of the calls in the code, e.g.,
        // the unused nodes of StaticDrops in `float s = StaticDrops(uv, t);`
        const auto var_def = std::dynamic_pointer_cast<VariableDefinition>(record->statement);
        const bool is_geomprop = var_def and var_def->modifiers().contains(TokenType::Geomprop);
        const bool is_value = (var_def and not is_geomprop) or std::dynamic_pointer_cast<VariableAssignment>(record->statement);
        if (is_value and record->is_complete and not statement.nodes.empty())
            return false;

        const bool is_loop = std::any_of(statement.children.begin(), statement.children.end(), [](const Instance* child) {
            return child->is_loop_iteration();
        });

        // the nodes of a statement can only be used outside of it through the values it assigns, or the values assigned
        // by the frames it is part of, e.g., the return value of the call whose body it is part of
        const bool is_return = std::dynamic_pointer_cast<ReturnStatement>(record->statement) != nullptr;

        return not has_external_uses(statement, [&](const mx::NodePtr& node, const string& output, const mx::InputPtr& input) {
            // the value was assigned to an input by the statement, e.g., `s.base_color = randomcolor();`
            if (input and DocumentHints::assignment(input) == statement.frame.id)
                return true;

            // the value is returned by the function whose body is the graph
            if (is_return and input == nullptr)
                return true;

            for (const Binding& binding : DocumentHints::bindings(node))
            {
                if (binding.output != output and not binding.output.empty() and not output.empty())
                    continue;

                // the new value of a variable that was incremented by the statement, e.g., `foo(++i);`
                const DocumentHints::CallRecord* call = binding.frame.kind == 'c' ? hints_.call(binding.frame.id) : nullptr;
                if (call and (call->function == "__inc__" or call->function == "__dec__"))
                    return true;

                for (const Instance* instance = this->instance(node); instance; instance = instance->parent)
                {
                    if (not (instance->frame == binding.frame))
                        continue;
                    if (is_within(statement, *instance))
                        return true;
                    // variables assigned in the body of a loop are still assigned after the loop
                    if (is_loop and binding.frame.kind == 's' and is_within(*instance, statement))
                        return true;
                    break;
                }
            }
            return false;
        });
    }

    void HintedGraph::validate_calls(Instance& frame)
    {
        for (Instance* child : frame.children)
        {
            validate_calls(*child);
            if (child->is_call() and not is_valid_call(*child))
                dissolve(*child);
        }
    }

    bool HintedGraph::is_valid_call(const Instance& call) const
    {
        const DocumentHints::CallRecord* record = hints_.call(call.frame.id);
        if (record == nullptr)
            return false;

        // functions defined in the document must be inline functions whose definition is known
        if (record->definition > 0 and not hints_.is_inline_function(record->definition))
            return false;

        const vector<BoundValue> values = call_values(call);

        // every argument must be found in the graph, either as the value of an input or as an out parameter
        for (const DocumentHints::Argument& arg : record->arguments)
        {
            const vector<mx::InputPtr> inputs = argument_inputs(call, arg.param);
            if (inputs.empty() and arg.code)
                continue;
            if (inputs.empty())
            {
                const bool is_out_param = std::any_of(values.begin(), values.end(), [&](const BoundValue& value) {
                    return not value.path.empty() and value.path.front() == arg.param;
                });
                if (not is_out_param)
                    return false;
                continue;
            }

            for (const mx::InputPtr& input : inputs)
            {
                if (input_value_key(input) != input_value_key(inputs.front()))
                    return false;
            }
        }

        // values can only enter the call through its parameters
        for (const mx::NodePtr& node : nodes(call))
        {
            for (const mx::InputPtr& input : node->getInputs())
            {
                const mx::NodePtr source = input->getConnectedNode();
                const bool is_external = (source and not (instance(source) and is_within(*instance(source), call))) or not input->getInterfaceName().empty();
                if (not is_external)
                    continue;

                const vector<DocumentHints::Source> sources = DocumentHints::sources(input);
                const bool is_argument = std::any_of(sources.begin(), sources.end(), [&](const DocumentHints::Source& source) {
                    return source.frame == call.frame;
                });
                if (not is_argument)
                    return false;
            }
        }

        // and they can only leave it through its return value and out parameters
        return not has_external_uses(call, [&](const mx::NodePtr& node, const string& output, const mx::InputPtr&) {
            return std::any_of(values.begin(), values.end(), [&](const BoundValue& value) {
                return value.value.node == node and (value.value.output == output or value.value.output.empty() or output.empty());
            });
        });
    }

    bool HintedGraph::is_removed(const size_t id) const
    {
        return contains(removed_statements_, id);
    }

    void HintedGraph::remove(Instance& instance)
    {
        if (instance.is_statement())
            removed_statements_.insert(instance.frame.id);

        for (const mx::NodePtr& node : nodes(instance))
            node_instances_.erase(node);

        Instance* parent = instance.parent;
        parent->children.erase(std::find(parent->children.begin(), parent->children.end(), &instance));
    }

    bool HintedGraph::has_external_uses(const Instance& frame, const std::function<bool(const mx::NodePtr&, const string&, const mx::InputPtr&)>& is_allowed) const
    {
        for (const mx::NodePtr& node : nodes(frame))
        {
            const auto it = uses_.find(node);
            if (it == uses_.end())
                continue;

            for (const Use& use : it->second)
            {
                const mx::NodePtr consumer_node = use.consumer->asA<mx::Node>();
                const Instance* consumer_instance = consumer_node ? instance(consumer_node) : nullptr;
                const bool is_internal = consumer_instance and is_within(*consumer_instance, frame);
                if (not is_internal and not is_allowed(node, use.output, use.input))
                    return true;
            }
        }
        return false;
    }
}
