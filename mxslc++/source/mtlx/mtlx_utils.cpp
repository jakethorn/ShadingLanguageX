//
// Created by jaket on 03/01/2026.
//

#include "mtlx_utils.h"

#include "runtime/Type.h"
#include "utils/str_utils.h"

string get_port_name(const string& port_name, const size_t i)
{
    return port_name + "__" + str(i);
}

void add_input(const mx::NodeDefPtr& node_def, const TypePtr& type, const string& name)
{
    if (type->has_fields())
    {
        for (size_t i = 0; i < type->field_count(); ++i)
            add_input(node_def, type->field_type(i), get_port_name(name, i));
    }
    else
    {
        if (node_def->getInput(name) == nullptr)
            node_def->addInput(name, type->name());
    }
}

mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name)
{
    mx::NodeDefPtr node_def = node_graph->getNodeDef();
    if (node_def->getOutput(name) == nullptr)
        node_def->addOutput(name, type->name());

    mx::OutputPtr output = node_graph->getOutput(name);
    if (output == nullptr)
        output = node_graph->addOutput(name, type->name());

    return output;
}

void set_interface(const mx::PortElementPtr& port, const string& interface_name)
{
    port->removeAttribute("value"s);
    port->setInterfaceName(interface_name);
}
