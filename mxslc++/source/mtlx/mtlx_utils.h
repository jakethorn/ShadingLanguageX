//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_MTLX_UTILS_H
#define FENNEC_MTLX_UTILS_H

#include "utils/common.h"

string get_port_name(const string& port_name, size_t i);

mx::InputPtr add_or_get_input(const mx::NodePtr& node, const string& type, const string& name);
mx::InputPtr add_or_get_input(const mx::NodePtr& node, const TypePtr& type, const string& name);
mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const TypePtr& type, const string& name);
void set_interface(const mx::PortElementPtr& port, const string& interface_name);

#endif //FENNEC_MTLX_UTILS_H
