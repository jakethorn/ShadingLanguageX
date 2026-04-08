//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_MTLX_UTILS_H
#define FENNEC_MTLX_UTILS_H

#include <MaterialXCore/Node.h>

#include "utils/common.h"

string as_string(const mx::NodePtr& node);
string port_name(const string& port_name, size_t i);
string port_name(const string& port_name, const string& s);

mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const string& name, const TypeInfoPtr& type);

#endif //FENNEC_MTLX_UTILS_H
