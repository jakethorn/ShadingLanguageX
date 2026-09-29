//
// Created by jaket on 28/09/2026.
//

#include "decompile/GraphComparator.h"

#include <functional>
#include <map>

#include "decompile/DocumentHints.h"

namespace mxslc::decompile
{
    namespace
    {
        string value_string(const mx::ValueElementPtr& element)
        {
            // e.g., "0.0, 0.0" and "0, 0" are the same value
            const mx::ValuePtr value = element->getValue();
            return value ? value->getValueString() : element->getValueString();
        }

        // "out" is the name of the output of single output nodes, so connecting to it by name is the same as not naming it
        string output_string(const mx::PortElementPtr& port)
        {
            const string& output = port->getOutputString();
            return output == "out" ? "" : output;
        }

        string attribute_string(const mx::ElementPtr& element, const vector<string>& ignored)
        {
            vector<string> attrs;
            for (const string& name : element->getAttributeNames())
                if (std::find(ignored.begin(), ignored.end(), name) == ignored.end() and not DocumentHints::is_hint(name))
                    attrs.push_back(name + "=" + element->getAttribute(name));
            std::sort(attrs.begin(), attrs.end());

            string result;
            for (const string& attr : attrs)
                result += attr + ";";
            return result;
        }

        string describe_graph(const mx::GraphElementPtr& graph)
        {
            unordered_map<string, string> signatures;

            const std::function<string(const string&)> signature = [&](const string& node_name) -> string {
                if (signatures.count(node_name))
                    return signatures.at(node_name);

                const mx::NodePtr node = graph->getNode(node_name);
                if (node == nullptr)
                    return "?" + node_name;

                signatures[node_name] = "<cycle>";
                vector<string> inputs;
                for (const mx::InputPtr& input : node->getInputs())
                {
                    string desc = input->getName() + ":" + input->getType() + "=";
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

                string desc = node->getCategory() + ":" + node->getType() + "[" + attribute_string(node, {"name", "type"}) + "](";
                for (const string& input : inputs)
                    desc += input + ",";
                desc += ")";

                // hash the description so that shared nodes do not grow the signatures exponentially
                const string hashed = std::to_string(std::hash<string>{}(desc));
                signatures[node_name] = hashed;
                return hashed;
            };

            vector<string> parts;
            for (const mx::NodePtr& node : graph->getNodes())
                parts.push_back("node " + signature(node->getName()));
            for (const mx::OutputPtr& output : graph->getOutputs())
            {
                const string source = output->getNodeName().empty() ? "interface(" + output->getInterfaceName() + ")" + value_string(output) : signature(output->getNodeName()) + "." + output_string(output);
                parts.push_back("output " + output->getName() + ":" + output->getType() + "=" + source);
            }
            for (const mx::InputPtr& input : graph->getInputs())
            {
                const string source = input->getNodeName().empty() ? value_string(input) : "node(" + input->getNodeName() + ")";
                parts.push_back("input " + input->getName() + ":" + input->getType() + "=" + source);
            }
            std::sort(parts.begin(), parts.end());

            string result;
            for (const string& part : parts)
                result += part + "\n";
            return result;
        }

        string describe_node_def(const mx::NodeDefPtr& node_def)
        {
            // the order of inputs is the order of the parameters, but the position of outputs does not matter
            string result = attribute_string(node_def, {"name"}) + "\n";
            for (const mx::InputPtr& input : node_def->getInputs())
                result += "input " + input->getName() + ":" + input->getType() + "=" + value_string(input) + "[" + attribute_string(input, {"name", "type", "value"}) + "]\n";

            vector<string> outputs;
            for (const mx::OutputPtr& output : node_def->getOutputs())
                outputs.push_back("output " + output->getName() + ":" + output->getType() + "[" + attribute_string(output, {"name", "type"}) + "]");
            std::sort(outputs.begin(), outputs.end());
            for (const string& output : outputs)
                result += output + "\n";

            return result;
        }

        std::map<string, string> describe(const mx::DocumentPtr& doc)
        {
            std::map<string, string> result;
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
    }

    vector<string> GraphComparator::differences(const mx::DocumentPtr& a, const mx::DocumentPtr& b)
    {
        const std::map<string, string> desc_a = describe(a);
        const std::map<string, string> desc_b = describe(b);

        vector<string> result;
        for (const auto& [key, value] : desc_a)
            if (not desc_b.count(key) or desc_b.at(key) != value)
                result.push_back(key);
        for (const auto& [key, value] : desc_b)
            if (not desc_a.count(key))
                result.push_back(key);
        return result;
    }
}
