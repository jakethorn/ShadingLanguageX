//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPH_UTILS_H
#define MXSLC_GRAPH_UTILS_H

#include <MaterialXCore/Document.h>
#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace mx = MaterialX;

// Compares two documents structurally. Nodes are identified by what they compute (their category, type, input values
// and the nodes they are connected to) and not by their names, so documents that only differ by the names or order of
// their nodes are equivalent.
class GraphComparator
{
public:
    // returns the names of the elements that differ, empty if the documents are equivalent
    static std::vector<std::string> differences(const mx::DocumentPtr& a, const mx::DocumentPtr& b)
    {
        const std::map<std::string, std::string> desc_a = describe(a);
        const std::map<std::string, std::string> desc_b = describe(b);

        std::vector<std::string> result;
        for (const auto& [key, value] : desc_a)
            if (not desc_b.count(key) or desc_b.at(key) != value)
                result.push_back(key);
        for (const auto& [key, value] : desc_b)
            if (not desc_a.count(key))
                result.push_back(key);
        return result;
    }

private:
    static std::string value_string(const mx::ValueElementPtr& element)
    {
        // e.g., "0.0, 0.0" and "0, 0" are the same value
        const mx::ValuePtr value = element->getValue();
        return value ? value->getValueString() : element->getValueString();
    }

    // "out" is the name of the output of single output nodes, so connecting to it by name is the same as not naming it
    static std::string output_string(const mx::PortElementPtr& port)
    {
        const std::string& output = port->getOutputString();
        return output == "out" ? "" : output;
    }

    static std::string attribute_string(const mx::ElementPtr& element, const std::vector<std::string>& ignored)
    {
        std::vector<std::string> attrs;
        for (const std::string& name : element->getAttributeNames())
            if (std::find(ignored.begin(), ignored.end(), name) == ignored.end())
                attrs.push_back(name + "=" + element->getAttribute(name));
        std::sort(attrs.begin(), attrs.end());

        std::string result;
        for (const std::string& attr : attrs)
            result += attr + ";";
        return result;
    }

    static std::string describe_graph(const mx::GraphElementPtr& graph)
    {
        std::unordered_map<std::string, std::string> signatures;

        const std::function<std::string(const std::string&)> signature = [&](const std::string& node_name) -> std::string {
            if (signatures.count(node_name))
                return signatures.at(node_name);

            const mx::NodePtr node = graph->getNode(node_name);
            if (node == nullptr)
                return "?" + node_name;

            signatures[node_name] = "<cycle>";
            std::vector<std::string> inputs;
            for (const mx::InputPtr& input : node->getInputs())
            {
                std::string desc = input->getName() + ":" + input->getType() + "=";
                if (not input->getNodeName().empty())
                    desc += "node(" + signature(input->getNodeName()) + ")." + output_string(input);
                else if (not input->getNodeGraphString().empty())
                    desc += "nodegraph(" + input->getNodeGraphString() + ")." + output_string(input);
                else if (not input->getInterfaceName().empty())
                    desc += "interface(" + input->getInterfaceName() + ")";
                else
                    desc += "value(" + value_string(input) + ")";
                desc += "[" + attribute_string(input, {"name", "type", "value", "nodename", "nodegraph", "output", "interfacename"}) + "]";
                inputs.push_back(desc);
            }
            std::sort(inputs.begin(), inputs.end());

            std::string desc = node->getCategory() + ":" + node->getType() + "[" + attribute_string(node, {"name", "type"}) + "](";
            for (const std::string& input : inputs)
                desc += input + ",";
            desc += ")";

            // hash the description so that shared nodes do not grow the signatures exponentially
            const std::string hashed = std::to_string(std::hash<std::string>{}(desc));
            signatures[node_name] = hashed;
            return hashed;
        };

        std::vector<std::string> parts;
        for (const mx::NodePtr& node : graph->getNodes())
            parts.push_back("node " + signature(node->getName()));
        for (const mx::OutputPtr& output : graph->getOutputs())
        {
            const std::string source = output->getNodeName().empty() ? "interface(" + output->getInterfaceName() + ")" + value_string(output) : signature(output->getNodeName()) + "." + output_string(output);
            parts.push_back("output " + output->getName() + ":" + output->getType() + "=" + source);
        }
        for (const mx::InputPtr& input : graph->getInputs())
        {
            const std::string source = input->getNodeName().empty() ? value_string(input) : "node(" + input->getNodeName() + ")";
            parts.push_back("input " + input->getName() + ":" + input->getType() + "=" + source);
        }
        std::sort(parts.begin(), parts.end());

        std::string result;
        for (const std::string& part : parts)
            result += part + "\n";
        return result;
    }

    static std::string describe_node_def(const mx::NodeDefPtr& node_def)
    {
        // the order of inputs is the order of the parameters, but the position of outputs does not matter
        std::string result = attribute_string(node_def, {"name"}) + "\n";
        for (const mx::InputPtr& input : node_def->getInputs())
            result += "input " + input->getName() + ":" + input->getType() + "=" + value_string(input) + "[" + attribute_string(input, {"name", "type", "value"}) + "]\n";

        std::vector<std::string> outputs;
        for (const mx::OutputPtr& output : node_def->getOutputs())
            outputs.push_back("output " + output->getName() + ":" + output->getType() + "[" + attribute_string(output, {"name", "type"}) + "]");
        std::sort(outputs.begin(), outputs.end());
        for (const std::string& output : outputs)
            result += output + "\n";

        return result;
    }

    static std::map<std::string, std::string> describe(const mx::DocumentPtr& doc)
    {
        std::map<std::string, std::string> result;
        result["document attributes"] = attribute_string(doc, {});
        result["document nodes"] = describe_graph(doc);
        for (const mx::ElementPtr& element : doc->getChildren())
        {
            if (const mx::NodeDefPtr node_def = element->asA<mx::NodeDef>())
                result["nodedef " + node_def->getName()] = describe_node_def(node_def);
            else if (const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>())
                result["nodegraph " + node_graph->getName()] = attribute_string(node_graph, {"name"}) + "\n" + describe_graph(node_graph);
        }
        return result;
    }
};

#endif //MXSLC_GRAPH_UTILS_H
