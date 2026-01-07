//
// Created by jaket on 04/01/2026.
//

#include "MtlXSerializer.h"

#include <cassert>
#include <functional>
#include <MaterialXFormat/XmlIo.h>
#include "CompileError.h"
#include "runtime/values/BasicValue.h"
#include "runtime/values/NodeValue.h"
#include "runtime/values/NullValue.h"
#include "runtime/values/Value.h"
#include "runtime/Function.h"
#include "runtime/ArgumentList.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "utils/template_utils.h"

#define BINARY_OP(ltype, op, rtype) \
    if (values[0]->is<ltype>() && values[1]->is<rtype>()) \
        return std::make_unique<BasicValue>(values[0]->get<ltype>() op values[1]->get<rtype>());

namespace
{
    using BasicValues = vector<const BasicValue*>;

    ValuePtr evaluate_add(const BasicValues& values)
    {
        BINARY_OP(int, +, int)
        BINARY_OP(int, +, float)
        BINARY_OP(float, +, int)
        BINARY_OP(float, +, float)
        return nullptr;
    }

    unordered_map<string, std::function<ValuePtr(const BasicValues&)>> constexpr_funcs {
        {"add"s, evaluate_add}
    };

    bool is_constexpr(const Function& func, const vector<ValuePtr>& args, BasicValues& values)
    {
        if (not contains(constexpr_funcs, func.name()))
            return false;

        for (const ValuePtr& arg : args)
            if (const BasicValue* value = dynamic_cast<BasicValue*>(arg.get()))
                values.push_back(value);
            else
                return false;

        return true;
    }

    ValuePtr evaluate_constexpr(const Function& func, const BasicValues& values)
    {
        return constexpr_funcs.at(func.name())(values);
    }
}

ValuePtr MtlXSerializer::write_function_call(const Function& func, const ArgumentList& args) const
{
    const vector<ValuePtr> values = args.evaluate();

    // check if the expression can be evaluated at compile-time
    if (BasicValues basic_values; is_constexpr(func, values, basic_values))
        if (ValuePtr value = evaluate_constexpr(func, basic_values))
            return value;

    mx::NodePtr node = graph()->addNode(func.name(), mx::EMPTY_STRING, func.type().name());

    for (size_t i = 0; i < args.size(); ++i)
    {
        const Value* value = values[i].get();
        const string& input_name = func.parameters()[args[i]].name();

        if (const auto* basic_value = dynamic_cast<const BasicValue*>(value))
        {
            const string& input_type = basic_value->type().name();
            std::visit(
                [&node, &input_name, &input_type](const auto& v) {
                    node->setInputValue(input_name, v, input_type);
                },
                basic_value->get()
            );
        }
        else if (const auto* node_value = dynamic_cast<const NodeValue*>(value))
        {
            node->setConnectedNode(input_name, node_value->node());
        }
        else if (const auto* null_value = dynamic_cast<const NullValue*>(value))
        {
            node->removeInput(input_name);
        }
    }

    return std::make_shared<NodeValue>(node);
}

void MtlXSerializer::write_function(const Function& func) const
{
    write_node_def(func);
    write_node_graph(func);
}

void MtlXSerializer::write_return(const ValuePtr& value) const
{
    if (graph()->getOutput("out"s) != nullptr)
    {
        throw CompileError{"Multiple return statements"s};
    }

    mx::OutputPtr output = graph()->addOutput("out"s, value->type().name());

    if (const auto* basic_value = dynamic_cast<const BasicValue*>(value.get()))
    {
        const string& output_type = basic_value->type().name();
        std::visit(
            [&output, &output_type](const auto& v) {
                output->setValue(v, output_type);
            },
            basic_value->get()
        );
    }
    else if (const auto* node_value = dynamic_cast<const NodeValue*>(value.get()))
    {
        output->setConnectedNode(node_value->node());
    }
    else if (const auto* null_value = dynamic_cast<const NullValue*>(value.get()))
    {
        throw CompileError{"Cannot return null value"s};
    }
}

void MtlXSerializer::save(const fs::path& filepath) const
{
    const string& xml = mx::writeToXmlString(doc_);
    save_file(filepath, xml);
}

namespace
{
    string node_def_name(const Function& func)
    {
        const string name = func.has_template_type() ? func.name() + "_" + func.template_type().name() : func.name();
        return "ND_" + name;
    }

    string node_graph_name(const Function& func)
    {
        const string name = func.has_template_type() ? func.name() + "_" + func.template_type().name() : func.name();
        return "NG_" + name;
    }
}

void MtlXSerializer::write_node_def(const Function& func) const
{
    const mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), func.type().name(), func.name());
    for (const Parameter& param : func.parameters())
    {
        mx::InputPtr input = node_def->addInput(param.name(), param.type().name());

        if (param.has_default_value())
        {
            const Value* value = param.evaluate().get();
            if (const auto* basic_value = dynamic_cast<const BasicValue*>(value))
            {
                std::visit([&input](const auto& v) {
                    input->setValue(v, input->getType());
                },
                basic_value->get());
            }
        }
    }
}

void MtlXSerializer::write_node_graph(const Function& func) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    enter_node_graph(node_graph);
    for (const StmtPtr& stmt : func.body())
        stmt->execute();
    exit_node_graph();
}

void MtlXSerializer::enter_node_graph(const mx::NodeGraphPtr& node_graph) const
{
    graphs_.push_back(node_graph);
}

void MtlXSerializer::exit_node_graph() const
{
    assert(graphs_.size() > 1);

    if (graph()->getOutputCount() == 0)
        throw CompileError{"Function does not return a value or access globals."s};

    graphs_.pop_back();
}

#undef BINARY_OP
