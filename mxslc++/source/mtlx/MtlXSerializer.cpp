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
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "utils/template_utils.h"

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

    bool is_constexpr(const Function& func, const vector<ValuePtr>& args, BasicValues& values)
    {
        if (not contains(constexpr_funcs, func.name()))
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

    ValuePtr evaluate_constexpr(const Function& func, const vector<ValuePtr>& args)
    {
        if (BasicValues basic_values; is_constexpr(func, args, basic_values))
            if (ValuePtr value = constexpr_funcs.at(func.name())(basic_values))
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

ValuePtr MtlXSerializer::write_node(const Function& func, const ArgumentList& args) const
{
    const vector<ValuePtr> values = args.evaluate();

    // evaluate as constexpr if possible
    if (ValuePtr value = evaluate_constexpr(func, values))
        return value;

    // node
    mx::NodePtr node = graph()->addNode(func.name(), mx::EMPTY_STRING, serialize_type(func.type()));

    // inputs
    for (size_t i = 0; i < args.size(); ++i)
    {
        const string& input_name = func.parameters()[args[i]].name();
        values[i]->set_as_node_input(node, input_name);
    }

    // outputs
    if (func.type()->has_fields())
    {
        for (size_t i = 0; i < func.type()->field_count(); ++i)
            node->addOutput(func.output_name(i), func.type()->field(i).type()->name());
    }

    return std::make_shared<NodeValue>(node, func.type());
}

void MtlXSerializer::write_node_def_graph(const Function& func) const
{
    write_node_def(func);
    write_node_graph(func);
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
        const string name = func.has_template_type() ? func.name() + "_" + func.template_type()->name() : func.name();
        return "ND_" + name;
    }

    string node_graph_name(const Function& func)
    {
        const string name = func.has_template_type() ? func.name() + "_" + func.template_type()->name() : func.name();
        return "NG_" + name;
    }
    
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

void MtlXSerializer::write_node_def(const Function& func) const
{
    const mx::NodeDefPtr node_def = doc_->addNodeDef(node_def_name(func), serialize_type(func.type()), func.name());
    for (const Parameter& param : func.parameters())
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

    if (func.type()->has_fields())
    {
        add_outputs_from_type(node_def, func.type(), "out"s);
    }
}

void MtlXSerializer::write_node_graph(const Function& func) const
{
    const mx::NodeGraphPtr node_graph = doc_->addNodeGraph(node_graph_name(func));
    enter_node_graph(node_graph);
    for (const StmtPtr& stmt : func.body())
        stmt->execute();
    if (func.return_expr() != nullptr)
    {
        func.return_expr()->init(func.type());
        const ValuePtr return_value = func.return_expr()->evaluate();
        return_value->set_as_node_graph_output(graph(), "out"s);
    }
    exit_node_graph();

    if (func.type() == TypeInfo::Void)
    {
        const mx::OutputPtr output = node_graph->addOutput("out"s, "integer"s);
        output->setValue(0, "integer"s);
    }
}

void MtlXSerializer::enter_node_graph(const mx::NodeGraphPtr& node_graph) const
{
    graphs_.push_back(node_graph);
}

void MtlXSerializer::exit_node_graph() const
{
    assert(graphs_.size() > 1);
    graphs_.pop_back();
}

#undef BINARY_OP
