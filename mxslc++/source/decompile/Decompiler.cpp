//
// Created by jaket on 19/06/2026.
//

#include "Decompiler.h"

#include <cassert>
#include <MaterialXFormat/XmlIo.h>

#include "utils/template_utils.h"

mxslc::Decompiler::Decompiler(const fs::path& src_path)
{
    document_ = mx::createDocument();
    mx::readFromXmlFile(document_, src_path.string());
}

mxslc::Decompiler::Decompiler(const string& source)
{
    document_ = mx::createDocument();
    mx::readFromXmlString(document_, source);
}

mxslc::Decompiler::Decompiler(mx::DocumentPtr document) : document_{std::move(document)}
{

}

string mxslc::Decompiler::decompile_document()
{
    code_ = "";
    decompiled_nodes_.clear();

    for (const mx::NodePtr& node : document_->getNodes())
        code_ += decompile_node(node) + "\n";
    return code_;
}

string mxslc::Decompiler::decompile_node(const string& node_name)
{
    return decompile_node(document_->getNode(node_name));
}

string mxslc::Decompiler::decompile_node(const mx::NodePtr& node)
{
    decompiled_nodes_.insert(node->getName());

    const string var_type = decompile_outputs(node->getActiveOutputs());
    const string var_name = node->getName();
    const string func_name = node->getCategory();
    const string func_args = decompile_node_inputs(node->getInputs());

    return var_type + " " + var_name + " = " + func_name + "(" + func_args + ");";
}

string mxslc::Decompiler::decompile_node_graph(const mx::NodeGraphPtr& node_graph)
{
    const mx::NodeDefPtr node_def = node_graph->getNodeDef();

    const string return_type = decompile_outputs(node_def->getActiveOutputs());
    const string func_name = node_def->getNodeString();


    return "void foo(){}";
}

string mxslc::Decompiler::decompile_outputs(const vector<mx::OutputPtr>& outputs)
{
    assert(not outputs.empty());

    if (outputs.size() == 1)
    {
        return outputs[0]->getType();
    }
    else
    {
        string result = "{";
        for (const mx::OutputPtr& output : outputs)
            result += output->getType() + " " + output->getName() + ", ";
        return result + "}";
    }
}

string mxslc::Decompiler::decompile_value(const mx::ValuePtr& value)
{
    return value->getTypeString() + "{" + value->getValueString() + "}";
}

string mxslc::Decompiler::decompile_interface_name(const string& interface_name)
{
    return interface_name;
}

string mxslc::Decompiler::decompile_node_name_and_output(const string& node_name, const string& output)
{
    return node_name + "." + output;
}

string mxslc::Decompiler::decompile_node_name(const string& node_name)
{
    if (not contains(decompiled_nodes_, node_name))
        code_ += decompile_node(node_name) + "\n";
    return node_name;
}

string mxslc::Decompiler::decompile_node_input(const mx::InputPtr& input)
{
    const string result = input->getName() + "=";
    if (input->hasValue())
        return result + decompile_value(input->getValue());
    if (input->hasInterfaceName())
        return result + decompile_interface_name(input->getInterfaceName());
    if (input->hasNodeName())
    {
        if (input->hasOutputString())
            return result + decompile_node_name_and_output(input->getNodeName(), input->getOutputString());
        else
            return result + decompile_node_name(input->getNodeName());
    }
    throw std::runtime_error("Cannot decompile input: " + input->asString());
}

string mxslc::Decompiler::decompile_node_inputs(const vector<mx::InputPtr>& inputs)
{
    string result;
    for (const mx::InputPtr& input : inputs)
        result += decompile_node_input(input) + ", ";
    return result;
}

string mxslc::Decompiler::decompile_node_def_input(const mx::InputPtr& input)
{
    string result = input->getType() + " " + input->getName();
    if (input->hasValue())
        result += " = " + decompile_value(input->getValue());
    return result;
}

string mxslc::Decompiler::decompile_node_def_inputs(const vector<mx::InputPtr>& inputs)
{
    string result;
    for (const mx::InputPtr& input : inputs)
        result += decompile_node_def_input(input) + ", ";
    return result;
}
