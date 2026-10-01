//
// Created by jaket on 30/09/2026.
//

#ifndef MXSLC_NODE_NAME_UTILS_H
#define MXSLC_NODE_NAME_UTILS_H

#include "common.h"

// Nodes are named after the variables that their values are assigned to, which lets the decompiler recreate the
// variables, e.g., the node of `float x = a + b;` is named x and the node of a later `x = x * 2.0;` is named var__x__1.
// Other nodes keep their temporary names, var__<n>.
namespace mxslc::serialize
{
    // names the nodes of the value of a variable when it is defined, e.g., x, or s__lo for the field lo of a struct s
    void name_nodes(const VarPtr& var);
    // names the nodes of a value that is assigned to a variable after its definition, e.g., var__x__1
    void name_assigned_nodes(const VarPtr& var, const VarPtr& value);
    // the values that are passed to the parameters of inline functions belong to the caller, so they keep their names
    void disable_node_naming(const VarPtr& var);
}

#endif //MXSLC_NODE_NAME_UTILS_H
