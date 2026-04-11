//
// Created by jaket on 03/01/2026.
//

#include "mtlx_utils.h"

#include "runtime/TypeInfo.h"
#include "utils/str_utils.h"

string as_string(const mx::NodePtr& node)
{
    string str = node->asString();
    for (const mx::InputPtr& i : node->getInputs())
        str += "\n    "s + i->asString();
    for (const mx::OutputPtr& i : node->getOutputs())
        str += "\n    "s + i->asString();
    return str;
}

string port_name(const string& port_name, const size_t i)
{
    return port_name + "__" + str(i);
}

mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const string& name, const TypeInfoPtr& type)
{
    mx::NodeDefPtr node_def = node_graph->getNodeDef();
    if (node_def->getOutput(name) == nullptr)
        node_def->addOutput(name, type->name());

    mx::OutputPtr output = node_graph->getOutput(name);
    if (output == nullptr)
        output = node_graph->addOutput(name, type->name());

    return output;
}
