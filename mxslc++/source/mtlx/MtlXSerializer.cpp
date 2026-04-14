//
// Created by jaket on 04/01/2026.
//

#include "MtlXSerializer.h"

#include <cassert>
#include <MaterialXFormat/XmlIo.h>
#include "CompileError.h"
#include "evaluate_mtlx.h"
#include "expressions/Expression.h"
#include "expressions/ExpressionFactory.h"
#include "values/NodeValue.h"
#include "values/Value.h"
#include "runtime/Function.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "values/ValueFactory.h"

namespace
{
    string serialize_type(const FuncPtr& func)
    {
        if (not func->nonlocal_outputs().empty())
            return TypeInfo::MultiOutput;
        if (func->type()->has_fields())
            return TypeInfo::MultiOutput;
        if (func->type() == TypeInfo::Void)
            return TypeInfo::Int;
        return func->type()->name();
    }
}

ValuePtr MtlXSerializer::write_node(const FuncPtr& func, const unordered_map<string, ValuePtr>& args) const
{
    // evaluate at compile-time if possible
    if (ValuePtr value = evaluate_now(func->name(), args))
        return value;

    // node
    const mx::NodePtr node = graph()->addNode(func->name(), mx::EMPTY_STRING, serialize_type(func));

    // inputs
    for (const auto& [input_name, value] : args)
    {
        value->set_as_node_input(node, input_name);
    }

    // inputs from nonlocal variables
    for (const auto& [input_name, var] : func->nonlocal_inputs())
    {
        var->value()->set_as_node_input(node, input_name);
    }

    // outputs
    for (size_t i = 0; i < func->type()->field_count(); ++i)
    {
        node->addOutput(func->output_name(i), func->type()->field_type(i)->name());
    }

    // outputs to nonlocal variables
    for (const auto& [output_name, var] : func->nonlocal_outputs())
    {
        var->set_value(ValueFactory::create_output_value(node, output_name, var->type()));
    }

    return ValueFactory::create_node_value(node, func->type());
}

namespace
{
    void add_inputs_from_type(const mx::NodeDefPtr& node_def, const TypeInfoPtr& type, const string& name)
    {
        if (type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
                add_inputs_from_type(node_def, type->field_type(i), port_name(name, i));
        }
        else
        {
            if (node_def->getInput(name) == nullptr)
                node_def->addInput(name, type->name());
        }
    }

    string nonlocal_out_name(const VarPtr& var)
    {
        return "nonlocal_out__" + var->name();
    }

    string nonlocal_in_name(const VarPtr& var)
    {
        return "nonlocal_in__" + var->name();
    }
}

ValuePtr MtlXSerializer::write_node_def_input(const VarPtr& var) const
{
    // in the case that a nonlocal variable has been assigned a local value
    // we grab that instead of nonlocal variables value
    const string output_name = nonlocal_out_name(var);
    const mx::OutputPtr& output = node_graph()->getOutput(output_name);
    if (output)
    {
        return ValueFactory::copy_value_from_port(output);
    }

    const string input_name = nonlocal_in_name(var);
    add_inputs_from_type(node_graph()->getNodeDef(), var->type(), input_name);
    graph_function()->add_nonlocal_input(input_name, var);
    return ValueFactory::create_interface_value(var);
}

void MtlXSerializer::write_node_def_output(const VarPtr& var, const ValuePtr& value) const
{
    const string output_name = nonlocal_out_name(var);
    value->set_as_node_graph_output(node_graph(), output_name);
    graph_function()->add_nonlocal_output(output_name, var);
}

void MtlXSerializer::write_node_def_graph(const FuncPtr& func) const
{
    const mx::NodeDefPtr node_def = write_node_def(func);
    write_node_graph(func, node_def);
}

void MtlXSerializer::save(const fs::path& filepath) const
{
    const string& xml = mx::writeToXmlString(doc_);
    save_file(filepath, xml);
}

mx::NodeDefPtr MtlXSerializer::write_node_def(const FuncPtr& func) const
{
    const mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), serialize_type(func), func->name());
    for (const Parameter* param : func->in_parameters())
    {
        if (param->has_default_value())
        {
            const ValuePtr value = param->evaluate();
            value->set_as_node_def_input(node_def, param->name());
        }
        else
        {
            add_inputs_from_type(node_def, param->type(), param->name());
        }
    }

    // node def outputs are added with the node graph outputs
    node_def->removeOutput("out"s);

    return node_def;
}

void MtlXSerializer::write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    node_graph->setNodeDef(node_def);

    enter_node_graph(node_graph, func);

    const ValuePtr return_value = func->invoke();

    if (not func->is_void())
    {
        return_value->set_as_node_graph_output(node_graph, "out"s);
    }
    else
    {
        const ValuePtr default_value = ValueFactory::create_default_value<int>();
        default_value->set_as_node_graph_output(node_graph, "out"s);
    }

    exit_node_graph();
}

string MtlXSerializer::node_def_name(const FuncPtr& func) const
{
    string name = "ND_" + func->name();
    if (func->has_template_type() and func->template_type()->has_name())
        name += "_" + func->template_type()->name();
    return doc_->createValidChildName(std::move(name));
}

string MtlXSerializer::node_graph_name(const FuncPtr& func) const
{
    string name = "NG_" + func->name();
    if (func->has_template_type() and func->template_type()->has_name())
        name += "_" + func->template_type()->name();
    return doc_->createValidChildName(std::move(name));
}

void MtlXSerializer::enter_node_graph(const mx::NodeGraphPtr& node_graph, const FuncPtr& func) const
{
    graphs_.push_back(GraphFunction{node_graph, func});
}

void MtlXSerializer::exit_node_graph() const
{
    assert(in_node_graph());
    graphs_.pop_back();
}

mx::NodeGraphPtr MtlXSerializer::node_graph() const
{
    if (mx::NodeGraphPtr node_graph = std::dynamic_pointer_cast<mx::NodeGraph>(graph()))
        return node_graph;

    throw CompileError{"Not a NodeGraph"s};
}
