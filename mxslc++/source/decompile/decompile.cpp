//
// Created by jaket on 19/06/2026.
//

#include "decompile.h"

#include <cassert>
#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include "utils/common.h"
#include "utils/io_utils.h"

namespace
{
    string decompile_node_outputs(const vector<mx::OutputPtr>& outputs)
    {
        assert(not outputs.empty());

        const string& var_name = outputs[0]->getNodeName();

        if (outputs.size() == 1)
        {
            return outputs[0]->getType() + " " + var_name;
        }
        else
        {
            string result = "{";
            for (const mx::OutputPtr& output : outputs)
                result += output->getType() + " " + output->getName() + ", ";
            return result + "}" + var_name;
        }
    }

    string decompile_value(const mx::ValuePtr& value)
    {
        return value->getTypeString() + "{" + value->getValueString() + "}";
    }

    string decompile_interface_name(const string& interface_name)
    {
        return interface_name;
    }

    string decompile_node_name_and_output(const string& node_name, const string& output)
    {
        return node_name + "." + output;
    }

    string decompile_node_name(const string& node_name)
    {
        return node_name;
    }

    string decompile_node_input(const mx::InputPtr& input)
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

    string decompile_node_inputs(const vector<mx::InputPtr>& inputs)
    {
        string result;
        for (const mx::InputPtr& input : inputs)
            result += decompile_node_input(input) + ", ";
        return result;
    }

    string decompile_node(const mx::NodePtr& node)
    {
        string result;
        result += decompile_node_outputs(node->getActiveOutputs());
        result += " = ";
        result += node->getCategory();
        result += "(";
        result += decompile_node_inputs(node->getInputs());
        result += ");";
        return result;
    }

    string decompile_document(const mx::DocumentPtr& doc)
    {
        string result;
        for (const mx::NodePtr& node : doc->getNodes())
            result += decompile_node(node) + "\n";
        return result;
    }
}

fs::path mxslc::decompile_to_file(const fs::path& src_path)
{
    const mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, src_path.string());

    const fs::path dst_path = fs::path{src_path}.replace_extension(".mxsl");
    const string code = decompile_document(doc);

    save_file(dst_path, code);
    return dst_path;
}
