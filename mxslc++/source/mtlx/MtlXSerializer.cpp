//
// Created by jaket on 04/01/2026.
//

#include "MtlXSerializer.h"

#include <cassert>
#include <MaterialXFormat/XmlIo.h>

#include "CompileError.h"
#include "mtlx_utils.h"
#include "evaluate_mtlx.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "runtime/ArgumentList.h"
#include "runtime/Type.h"
#include "runtime/Function.h"
#include "values/InterfaceValue.h"
#include "values/ValueFactory.h"
#include "utils/io_utils.h"

namespace
{
    string serialize_type(const FuncPtr& func)
    {
        const vector<mx::OutputPtr> outputs = func->node_def()->getActiveOutputs();
        assert(not outputs.empty());
        if (outputs.size() == 1)
            return outputs[0]->getType();
        if (outputs.size() > 1)
            return "multioutput";
        throw CompileError{"Unable to serialize function type"s};
    }

    void add_outputs_to_node_def(const mx::NodeDefPtr& node_def, const TypePtr& type, const string& name = "out")
    {
        if (type->is_void())
            return;

        if (type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                add_outputs_to_node_def(node_def, type->field_type(i), get_port_name(name, i));
            }
        }
        else
        {
            node_def->addOutput(name, type->name());
        }
    }

    string node_name(const FuncPtr& func)
    {
        if (not func->is_defined())
            return func->name();

        const Scope* scope = &Runtime::get().scope().get_defining_scope(func);

        string result = "";
        while (true)
        {
            if (not scope->is_inline())
            {
                const FuncPtr& defining_func = scope->node_graph().second;
                result += node_name(defining_func) + "_";
                break;
            }
            if (not scope->has_parent())
                break;
            scope = &scope->parent();
        }
        if (func->has_class_type())
            result += func->class_type()->name() + "_";
        result += func->name();
        if (func->has_template_type())
            result += "_" + func->template_type()->name();
        return result;
    }

    string nonlocal_out_name(const VarPtr& var)
    {
        return "nonlocal_out__" + var->name();
    }

    string nonlocal_in_name(const VarPtr& var)
    {
        return "nonlocal_in__" + var->name();
    }

    void write_default_output(const mx::NodeDefPtr& node_def, const mx::NodeGraphPtr& node_graph)
    {
        // this happens if the function is void, has no out or ref parameters and doesn't mutate a nonlocal variable
        if (node_def and node_def->getActiveOutputs().empty())
        {
            node_def->addOutput("out", TypeName::Int);
            node_graph->addOutput("out", TypeName::Int)->setValueString("0");
        }
    }
}

VarPtr MtlXSerializer::write_node(const FuncPtr& func, const ArgumentList& args, const AttributeList& attrs) const
{
    return write_node(nullptr, func, args, attrs);
}

VarPtr MtlXSerializer::write_node(const VarPtr& instance, const FuncPtr& func, const ArgumentList& args, const AttributeList& attrs) const
{
    ParameterValues input_values = args.evaluate(func->parameters());

    if (reduce_graph_)
    {
        if (VarPtr value = evaluate_now(func->return_type(), func->name(), input_values))
            return value;
    }

    const mx::GraphElementPtr& graph = scope().graph();
    const mx::NodePtr node = graph->addNode(node_name(func), mx::EMPTY_STRING, serialize_type(func));

    for (const auto& [param, input_value] : input_values)
    {
        const Argument* arg = args[param];
        AttributeList input_attrs = arg != nullptr ? args[param]->attributes() : AttributeList{};

        if (param.is_in())
        {
            write_node_input(node, param.name(), input_value, input_attrs);
        }

        if (param.is_out())
        {
            const VarPtr output = ValueFactory::create_output_value(node, param.type(), "out__" + param.name(), input_attrs);
            input_value->copy(output);
        }
    }

    // inputs from and outputs to instance
    if (instance != nullptr)
    {
        assert(instance->type() == func->class_type());

        write_node_input(node, "this", instance);

        if (func->mutates_instance())
        {
            const VarPtr output = ValueFactory::create_output_value(node, instance->type(), "out_this");
            instance->copy(output);
        }
    }

    // inputs from nonlocal variables
    for (const VarPtr& var : func->nonlocal_inputs())
    {
        write_node_input(node, nonlocal_in_name(var), var);
    }

    // outputs to nonlocal variables
    for (const VarPtr& var : func->nonlocal_outputs())
    {
        const VarPtr nonlocal_output = ValueFactory::create_output_value(node, var->type(), nonlocal_out_name(var));
        var->copy(nonlocal_output);
    }

    attrs.add_to(node);

    return ValueFactory::create_node_value(node, func);
}

void MtlXSerializer::write_node_def_graph(const FuncPtr& func, const AttributeList& attrs) const
{
    runtime().enter_scope();

    if (func->is_parameterless())
    {
        const mx::NodeGraphPtr node_graph = write_node_graph(func, nullptr);
        attrs.add_to(node_graph);
    }
    else
    {
        const mx::NodeDefPtr node_def = write_node_def(func);
        write_node_graph(func, node_def);
        attrs.add_to(node_def);
    }

    runtime().exit_scope();
}

ValuePtr MtlXSerializer::write_node_def_input(const VarPtr& var) const
{
    const auto& [node_graph, func] = scope().node_graph();

    if (func->is_parameterless())
        throw CompileError{"Cannot access nonlocal variables in parameterless function"};

    // in the case that a nonlocal variable has been assigned a local value
    // we grab that instead of nonlocal variables value
    const string output_name = nonlocal_out_name(var);
    const mx::OutputPtr& output = node_graph->getOutput(output_name);
    if (output)
    {
        return ValueFactory::copy_value_from_port(output);
    }

    const string input_name = nonlocal_in_name(var);
    write_node_def_input(node_graph->getNodeDef(), input_name, var->type());
    func->add_nonlocal_input(var);
    return std::make_shared<InterfaceValue>(var->type(), input_name);
}

void MtlXSerializer::write_node_def_output(const VarPtr& var, const ValuePtr& value) const
{
    const auto& [node_graph, func] = scope().node_graph();

    if (func->is_parameterless())
        throw CompileError{"Cannot access nonlocal variables in parameterless function"};

    const string output_name = nonlocal_out_name(var);
    value->set_as_node_graph_output(node_graph, output_name);
    func->add_nonlocal_output(var);
}

string MtlXSerializer::xml() const
{
    return mx::writeToXmlString(doc_);
}

void MtlXSerializer::save(const fs::path& dst_path) const
{
    save_file(dst_path, xml());
}

mx::NodeDefPtr MtlXSerializer::write_node_def(const FuncPtr& func) const
{
    mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), TypeName::Int, node_name(func));
    node_def->removeOutput("out");
    add_outputs_to_node_def(node_def, func->return_type());

    for (const Parameter& param : func->parameters())
    {
        if (param.is_in())
        {
            const VarPtr in_var = param.has_default_value() ? param.evaluate() : ValueFactory::create_default_value(param.type());
            write_node_def_input(node_def, param.name(), in_var, param.attributes());

            const VarPtr interface = ValueFactory::create_interface_value(param.type(), param.name());
            interface->set_modifiers(param.modifiers().without(TokenType::Ref, TokenType::Out));
            interface->add_to_scope(param.name());
        }
        else
        {
            const VarPtr out_var = param.has_default_value() ? param.evaluate() : ValueFactory::create_default_value(param.type());
            out_var->set_modifiers(param.modifiers().without(TokenType::Ref, TokenType::Out));
            out_var->add_to_scope(param.name());
        }
    }

    add_instance_to_scope(func, node_def);

    func->set_node_def(node_def);
    return node_def;
}

mx::NodeGraphPtr MtlXSerializer::write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    node_graph->setNodeDef(node_def);

    scope().set_graph(node_graph, func);
    const VarPtr instance_copy = copy_instance(func);
    const VarPtr return_value = func->invoke();

    if (not func->is_void())
    {
        write_node_graph_output(node_graph, "out", return_value);
    }

    for (const Parameter& param : func->parameters())
    {
        if (param.is_out())
        {
            const VarPtr out_value = scope().get_variable(param.name());
            write_node_graph_output(node_graph, "out__" + param.name(), out_value, param.attributes());
        }
    }

    update_instance(func, node_graph, instance_copy);
    write_default_output(node_def, node_graph);

    func->set_node_graph(node_graph);
    return node_graph;
}

void MtlXSerializer::add_instance_to_scope(const FuncPtr& func, const mx::NodeDefPtr& node_def) const
{
    if (func->has_class_type() and not func->is_parameterless())
    {
        write_node_def_input(node_def, "this", func->class_type());

        const VarPtr instance = ValueFactory::create_interface_value(func->class_type(), "this");
        instance->set_modifiers(TokenType::Mutable);
        instance->add_to_scope("this");
    }
}

VarPtr MtlXSerializer::copy_instance(const FuncPtr& func) const
{
    if (func->has_class_type() and not func->is_parameterless())
        return scope().get_variable("this")->copy();
    else
        return nullptr;
}

void MtlXSerializer::update_instance(const FuncPtr& func, const mx::NodeGraphPtr& node_graph, const VarPtr& original_instance) const
{
    if (func->has_class_type() and not func->is_parameterless())
    {
        const VarPtr instance = scope().get_variable("this");
        func->set_mutates_instance(not instance->equals(original_instance));
        if (func->mutates_instance())
            write_node_graph_output(node_graph, "out__this", instance);
    }
}

void MtlXSerializer::write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var) const
{
    write_node_input(node, input_name, var, AttributeList{});
}

void MtlXSerializer::write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var, const AttributeList& attrs) const
{
    if (var->has_value())
    {
        var->value()->set_as_node_input(node, input_name);
        attrs.add_to(node, input_name);
    }
    else
    {
        for (size_t i = 0; i < var->child_count(); ++i)
        {
            write_node_input(node, get_port_name(input_name, i), var->child(i), attrs);
        }
    }
}

void MtlXSerializer::write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var) const
{
    write_node_graph_output(node_graph, output_name, var, AttributeList{});
}

void MtlXSerializer::write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var, const AttributeList& attrs) const
{
    if (var->has_value())
    {
        var->value()->set_as_node_graph_output(node_graph, output_name);
        if (const mx::NodeDefPtr node_def = node_graph->getNodeDef())
            attrs.add_to(node_def, output_name);
    }
    else
    {
        for (size_t i = 0; i < var->child_count(); ++i)
        {
            write_node_graph_output(node_graph, get_port_name(output_name, i), var->child(i), attrs);
        }
    }
}

void MtlXSerializer::write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const TypePtr& type) const
{
    if (type->has_fields())
    {
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            write_node_def_input(node_def, get_port_name(input_name, i), type->field_type(i));
        }
    }
    else
    {
        if (node_def->getInput(input_name) == nullptr)
            node_def->addInput(input_name, type->name());
    }
}

void MtlXSerializer::write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const VarPtr& var) const
{
    write_node_def_input(node_def, input_name, var, AttributeList{});
}

void MtlXSerializer::write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const VarPtr& var, const AttributeList& attrs) const
{
    if (var->has_value())
    {
        var->value()->set_as_node_def_input(node_def, input_name);
        attrs.add_to(node_def, input_name);
    }
    else
    {
        for (size_t i = 0; i < var->child_count(); ++i)
        {
            write_node_def_input(node_def, get_port_name(input_name, i), var->child(i), attrs);
        }
    }
}

string MtlXSerializer::node_def_name(const FuncPtr& func) const
{
    return doc_->createValidChildName("ND_" + node_name(func));
}

string MtlXSerializer::node_graph_name(const FuncPtr& func) const
{
    return doc_->createValidChildName("NG_" + node_name(func));
}
