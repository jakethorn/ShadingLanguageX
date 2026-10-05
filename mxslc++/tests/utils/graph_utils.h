//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_GRAPH_UTILS_H
#define MXSLC_GRAPH_UTILS_H

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <MaterialXCore/Document.h>

#include "common.h"

namespace test_utils
{
    using namespace mxslc;

    // Compares two documents structurally. Nodes are identified by what they compute (their category, type, input values
    // and the nodes they are connected to) and not by their names, so documents that only differ by the names or order of
    // their nodes are equivalent.
    class GraphComparator
    {
    public:
        // returns the names of the elements that differ, empty if the documents are equivalent
        static vector<string> find_differences(const mx::DocumentPtr& a, const mx::DocumentPtr& b);
    };

    namespace graph_comparator_detail
    {
        inline string get_value_string(const mx::ValueElementPtr& element)
        {
            // e.g., "0.0, 0.0" and "0, 0" are the same value
            const mx::ValuePtr value = element->getValue();
            return value ? value->getValueString() : element->getValueString();
        }

        // "out" is the name of the output of single output nodes, so connecting to it by name is the same as not naming it
        inline string get_output_string(const mx::PortElementPtr& port)
        {
            const string& output = port->getOutputString();
            return output == "out" ? "" : output;
        }

        inline string get_attribute_string(const mx::ElementPtr& element, const vector<string>& ignored)
        {
            vector<string> attrs;
            for (const string& name : element->getAttributeNames())
                if (std::find(ignored.begin(), ignored.end(), name) == ignored.end())
                    attrs.push_back(name + "=" + element->getAttribute(name));
            std::sort(attrs.begin(), attrs.end());

            string result;
            for (const string& attr : attrs)
                result += attr + ";";
            return result;
        }

        inline string describe_graph(const mx::GraphElementPtr& graph)
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
                        desc += "node(" + signature(input->getNodeName()) + ")." + get_output_string(input);
                    else if (not input->getNodeGraphString().empty())
                        desc += "nodegraph(" + input->getNodeGraphString() + ")." + get_output_string(input);
                    else if (not input->getInterfaceName().empty())
                        desc += "interface(" + input->getInterfaceName() + ")";
                    else
                        desc += "value(" + get_value_string(input) + ")";
                    desc += "[" + get_attribute_string(input, {"name", "type", "value", "nodename", "nodegraph", "output", "interfacename"}) + "]";
                    inputs.push_back(desc);
                }
                std::sort(inputs.begin(), inputs.end());

                string desc = node->getCategory() + ":" + node->getType() + "[" + get_attribute_string(node, {"name", "type"}) + "](";
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
                const string source = output->getNodeName().empty() ? "interface(" + output->getInterfaceName() + ")" + get_value_string(output) : signature(output->getNodeName()) + "." + get_output_string(output);
                parts.push_back("output " + output->getName() + ":" + output->getType() + "=" + source);
            }
            for (const mx::InputPtr& input : graph->getInputs())
            {
                string source = get_value_string(input);
                if (not input->getNodeName().empty())
                    source = "node(" + input->getNodeName() + ")";
                else if (not input->getNodeGraphString().empty())
                    source = "nodegraph(" + input->getNodeGraphString() + ")." + get_output_string(input);
                parts.push_back("input " + input->getName() + ":" + input->getType() + "=" + source);
            }
            std::sort(parts.begin(), parts.end());

            string result;
            for (const string& part : parts)
                result += part + "\n";
            return result;
        }

        // the parameters of a function, whose order matters, as opposed to the nonlocal variables that it reads, which are
        // added in the order that the function uses them
        inline bool is_parameter(const mx::InputPtr& input)
        {
            return input->getName().rfind("nonlocal_in__", 0) != 0;
        }

        inline string describe_node_def(const mx::NodeDefPtr& node_def)
        {
            // the order of parameters matters, but the position of nonlocal variables and outputs does not
            string result = get_attribute_string(node_def, {"name"}) + "\n";
            vector<string> nonlocal_inputs;
            for (const mx::InputPtr& input : node_def->getInputs())
            {
                const string desc = "input " + input->getName() + ":" + input->getType() + "=" + get_value_string(input) + "[" + get_attribute_string(input, {"name", "type", "value"}) + "]\n";
                if (is_parameter(input))
                    result += desc;
                else
                    nonlocal_inputs.push_back(desc);
            }
            std::sort(nonlocal_inputs.begin(), nonlocal_inputs.end());
            for (const string& input : nonlocal_inputs)
                result += input;

            vector<string> outputs;
            for (const mx::OutputPtr& output : node_def->getOutputs())
                outputs.push_back("output " + output->getName() + ":" + output->getType() + "[" + get_attribute_string(output, {"name", "type"}) + "]");
            std::sort(outputs.begin(), outputs.end());
            for (const string& output : outputs)
                result += output + "\n";

            return result;
        }

        // node defs are identified by their function and signature rather than their names, because overloads are named
        // in the order that they are defined, e.g., ND_foo and ND_foo2
        inline string get_signature(const mx::NodeDefPtr& node_def)
        {
            string result = node_def->getNodeString() + "(";
            for (const mx::InputPtr& input : node_def->getInputs())
            {
                if (is_parameter(input))
                    result += input->getType() + ",";
            }

            vector<string> outputs;
            for (const mx::OutputPtr& output : node_def->getOutputs())
                outputs.push_back(output->getName() + ":" + output->getType());
            std::sort(outputs.begin(), outputs.end());

            result += ")->(";
            for (const string& output : outputs)
                result += output + ",";
            return result + ")";
        }

        inline std::map<string, string> describe(const mx::DocumentPtr& doc)
        {
            std::map<string, string> result;
            result["document attributes"] = get_attribute_string(doc, {});
            result["document nodes"] = describe_graph(doc);
            for (const mx::ElementPtr& element : doc->getChildren())
            {
                if (const mx::NodeDefPtr node_def = element->asA<mx::NodeDef>())
                {
                    result["nodedef " + get_signature(node_def)] = describe_node_def(node_def);
                }
                else if (const mx::NodeGraphPtr node_graph = element->asA<mx::NodeGraph>())
                {
                    const mx::NodeDefPtr implemented = node_graph->getNodeDef();
                    const string key = implemented ? "nodegraph of " + get_signature(implemented) : "nodegraph " + node_graph->getName();
                    result[key] = get_attribute_string(node_graph, {"name", "nodedef"}) + "\n" + describe_graph(node_graph);
                }
            }
            return result;
        }
    }

    inline vector<string> GraphComparator::find_differences(const mx::DocumentPtr& a, const mx::DocumentPtr& b)
    {
        using namespace graph_comparator_detail;
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

using test_utils::GraphComparator;

#endif //MXSLC_GRAPH_UTILS_H
