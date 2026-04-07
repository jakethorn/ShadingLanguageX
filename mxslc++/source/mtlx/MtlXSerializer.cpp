//
// Created by jaket on 04/01/2026.
//

#include "MtlXSerializer.h"

#include <cassert>
#include <functional>
#include <MaterialXFormat/XmlIo.h>
#include "CompileError.h"
#include "expressions/Expression.h"
#include "values/BasicValue.h"
#include "values/NodeValue.h"
#include "values/Value.h"
#include "runtime/Function.h"
#include "runtime/ArgumentList.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "utils/template_utils.h"
#include "values/InterfaceValue.h"
#include "values/ValueFactory.h"

#define BINARY_OP(ltype, op, rtype) \
    if (values[0]->is<ltype>() && values[1]->is<rtype>()) \
        return std::make_shared<BasicValue>(values[0]->get<ltype>() op values[1]->get<rtype>());

namespace
{
    using BasicValues = vector<shared_ptr<BasicValue>>;

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

    bool is_constexpr(const FuncPtr& func, const vector<ValuePtr>& args, BasicValues& values)
    {
        if (not contains(constexpr_funcs, func->name()))
            return false;

        for (const ValuePtr& arg : args)
        {
            if (shared_ptr<BasicValue> value = std::dynamic_pointer_cast<BasicValue>(arg))
                values.push_back(value);
            else
                return false;
        }

        return true;
    }

    ValuePtr evaluate_constexpr(const FuncPtr& func, const vector<ValuePtr>& args)
    {
        if (BasicValues basic_values; is_constexpr(func, args, basic_values))
            if (ValuePtr value = constexpr_funcs.at(func->name())(basic_values))
                return value;
        return nullptr;
    }

    string serialize_type(const TypeInfoPtr& type)
    {
        if (type->has_fields())
            return TypeInfo::MultiOutput;
        if (type == TypeInfo::Void)
            return TypeInfo::Int;
        return type->name();
    }
}

ValuePtr MtlXSerializer::write_node(const FuncPtr& func, const ArgumentList& args) const
{
    const vector<ValuePtr> values = args.evaluate();

    // evaluate as constexpr if possible
    if (ValuePtr value = evaluate_constexpr(func, values))
        return value;

    // node
    const mx::NodePtr node = graph()->addNode(func->name(), mx::EMPTY_STRING, serialize_type(func->type()));

    // inputs
    for (size_t i = 0; i < args.size(); ++i)
    {
        const string& input_name = func->parameters()[args[i]].name();
        values[i]->set_as_node_input(node, input_name);
    }

    for (const auto& [input_name, var] : func->nonlocal_inputs())
    {
        var->value()->set_as_node_input(node, input_name);
    }

    // outputs
    for (size_t i = 0; i < func->type()->field_count(); ++i)
    {
        node->addOutput(func->output_name(i), func->type()->field_type(i)->name());
    }

    for (const auto& [output_name, var] : func->nonlocal_outputs())
    {
        mx::OutputPtr output = node->addOutput(output_name, var->type()->name());
        var->set_value(std::make_shared<OutputValue>(output, var->type()));
    }

    return ValueFactory::create_node_value(node, func->type());
}

ValuePtr MtlXSerializer::write_node_def_input(const VarPtr& var) const
{
    const mx::NodeDefPtr node_def = node_graph()->getNodeDef();
    const string input_name = node_graph()->createValidChildName("in__"s);
    node_def->addInput(input_name, var->type()->name());
    graph_function()->add_nonlocal_input(input_name, var);
    return std::make_shared<InterfaceValue>(input_name, var->type());
}

void MtlXSerializer::write_node_def_output(const VarPtr& var, const ValuePtr& value) const
{
    const mx::NodeDefPtr node_def = node_graph()->getNodeDef();
    const string output_name = node_graph()->createValidChildName("out__"s);
    node_def->addOutput(output_name, var->type()->name());
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
            node_def->addInput(name, type->name());
        }
    }

    void add_outputs_from_type(const mx::NodeDefPtr& node_def, const TypeInfoPtr& type, const string& name)
    {
        if (type->has_fields())
        {
            for (size_t i = 0; i < type->field_count(); ++i)
                add_outputs_from_type(node_def, type->field_type(i), port_name(name, i));
        }
        else
        {
            node_def->addOutput(name, type->name());
        }
    }
}

mx::NodeDefPtr MtlXSerializer::write_node_def(const FuncPtr& func) const
{
    const mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), serialize_type(func->type()), func->name());
    for (const Parameter& param : func->parameters())
    {
        if (param.has_default_value())
        {
            const ValuePtr value = param.evaluate();
            value->set_as_node_def_input(node_def, param.name());
        }
        else
        {
            add_inputs_from_type(node_def, param.type(), param.name());
        }
    }

    if (func->type()->has_fields())
    {
        add_outputs_from_type(node_def, func->type(), "out"s);
    }

    return node_def;
}

void MtlXSerializer::write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    node_graph->setNodeDef(node_def);

    enter_node_graph(node_graph, func);

    for (const StmtPtr& stmt : func->body())
        stmt->execute();

    if (func->return_expr() != nullptr)
    {
        func->return_expr()->init(func->type());
        const ValuePtr return_value = func->return_expr()->evaluate();
        return_value->set_as_node_graph_output(graph(), "out"s);
    }

    exit_node_graph();

    if (func->type() == TypeInfo::Void)
    {
        const mx::OutputPtr output = node_graph->addOutput("out"s, "integer"s);
        output->setValue(0, "integer"s);
    }
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

#undef BINARY_OP
