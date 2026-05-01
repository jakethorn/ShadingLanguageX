//
// Created by jaket on 04/01/2026.
//

#include "MtlXSerializer.h"

#include <cassert>
#include <MaterialXFormat/XmlIo.h>
#include "CompileError.h"
#include "evaluate_mtlx.h"
#include "mtlx_utils.h"
#include "expressions/Expression.h"
#include "expressions/ExpressionFactory.h"
#include "runtime/ArgumentList.h"
#include "values/NodeValue.h"
#include "values/Value.h"
#include "runtime/Function.h"
#include "runtime/TypeInfo.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "values/InterfaceValue.h"
#include "values/ValueFactory.h"

namespace
{
    string serialize_type(const TypeInfoPtr& type)
    {
        if (type->has_fields())
            return TypeInfo::MultiOutput;
        if (type == TypeInfo::Void)
            return TypeInfo::Int;
        return type->name();
    }

    string serialize_type(const FuncPtr& func)
    {
        if (not func->nonlocal_outputs().empty())
            return TypeInfo::MultiOutput;
        return serialize_type(func->return_type());
    }

    void add_inputs_to_node_def(const mx::NodeDefPtr& node_def, const TypeInfoPtr& type, const string& name)
    {
        if (type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                add_inputs_to_node_def(node_def, type->field_type(i), port_name(name, i));
            }
        }
        else
        {
            if (node_def->getInput(name) == nullptr)
                node_def->addInput(name, serialize_type(type));
        }
    }

    void add_outputs_to_node_def(const mx::NodeDefPtr& node_def, const TypeInfoPtr& type, const string& name = "out"s)
    {
        if (type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                add_outputs_to_node_def(node_def, type->field_type(i), port_name(name, i));
            }
        }
        else
        {
            node_def->addOutput(name, serialize_type(type));
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

VarPtr MtlXSerializer::write_node(const FuncPtr& func, const ArgumentList& args) const
{
    vector<pair<const Parameter&, VarPtr>> arg_values = args.evaluate(func->parameters());

    // evaluate at compile-time if possible
    if (VarPtr value = evaluate_now(func->name(), arg_values))
        return value;

    const mx::NodePtr node = graph()->addNode(func->name(), mx::EMPTY_STRING, serialize_type(func));

    for (const auto& [param, arg_value] : arg_values)
    {
        if (param.is_in())
        {
            arg_value->set_as_node_input(node, param.name());
        }

        if (param.is_out())
        {
            const VarPtr output = ValueFactory::create_output_value(node, param.type(), "out__" + param.name());
            arg_value->copy_value(output);
        }
    }

    // inputs from nonlocal variables
    for (const auto& [input_name, var] : func->nonlocal_inputs())
    {
        var->set_as_node_input(node, input_name);
    }

    // outputs to nonlocal variables
    for (const auto& [output_name, var] : func->nonlocal_outputs())
    {
        const VarPtr nonlocal_output = ValueFactory::create_output_value(node, var->type(), output_name);
        var->copy_value(nonlocal_output);
    }

    return ValueFactory::create_node_value(node, func);
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
    add_inputs_to_node_def(node_graph()->getNodeDef(), var->type(), input_name);
    graph_function()->add_nonlocal_input(input_name, var);
    return std::make_shared<InterfaceValue>(var->type(), input_name);
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

string MtlXSerializer::xml() const
{
    return mx::writeToXmlString(doc_);
}

void MtlXSerializer::save(const fs::path& filepath) const
{
    save_file(filepath, xml());
}

mx::NodeDefPtr MtlXSerializer::write_node_def(const FuncPtr& func) const
{
    mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), serialize_type(func), func->name());
    node_def->removeOutput("out"s);
    add_outputs_to_node_def(node_def, func->return_type());

    for (const Parameter& param : func->parameters())
    {
        if (param.is_in())
        {
            // create node def input
            const VarPtr input = param.has_default_value() ? param.evaluate() : ValueFactory::create_default_value(param.type());
            input->set_as_node_def_input(node_def, param.name());

            // create interface value
            const VarPtr interface = ValueFactory::create_interface_value(param.type(), param.name());
            interface->set_modifiers(param.modifiers().without(TokenType::Ref, TokenType::Out));
            interface->add_to_scope(param.name());
        }
        else
        {
            const VarPtr output = param.has_default_value() ? param.evaluate() : ValueFactory::create_default_value(param.type());
            output->set_modifiers(param.modifiers().without(TokenType::Ref, TokenType::Out));
            output->add_to_scope(param.name());
        }
    }

    func->set_node_def(node_def);
    return node_def;
}

void MtlXSerializer::write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    node_graph->setNodeDef(node_def);

    enter_node_graph(node_graph, func);

    const VarPtr return_value = func->invoke();
    if (func->is_void())
    {
        const VarPtr default_value = ValueFactory::create_default_value<int>();
        default_value->set_as_node_graph_output(node_graph, "out"s);
    }
    else
    {
        return_value->set_as_node_graph_output(node_graph, "out"s);
    }

    for (const Parameter& param : func->parameters())
    {
        if (param.is_out())
        {
            const VarPtr output = Runtime::get().scope().get_variable(param.name());
            output->set_as_node_graph_output(node_graph, "out__" + param.name());
        }
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
