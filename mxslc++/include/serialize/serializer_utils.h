//
// Created by jaket on 10/07/2026.
//

#ifndef MXSLC_SERIALIZER_UTILS_H
#define MXSLC_SERIALIZER_UTILS_H

#include <MaterialXCore/Node.h>

#include "common.h"
#include "primitive_t.h"

namespace mxslc::runtime
{
    class AttributeList;
}

namespace mxslc::serialize_utils
{
    VarPtr create_interface_value(TypePtr type, const string& name);
    VarPtr create_node_value(mx::NodePtr node, const mx::NodeDefPtr& node_def, TypePtr type);
    VarPtr create_node_value(mx::NodePtr node, const FuncPtr& func);
    VarPtr create_node_graph_value(mx::NodeGraphPtr node_graph, TypePtr type);
    VarPtr create_node_graph_value(const FuncPtr& func);
    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const string& output_name);
    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const string& output_name, const AttributeList& attrs);
    VarPtr create_node_output_value(mx::NodePtr node, TypePtr type, const vector<string>& output_names);
    VarPtr create_node_graph_output_value(mx::NodeGraphPtr node_graph, TypePtr type, const string& output_name);
    VarPtr create_basic_value(primitive_t value);
    VarPtr create_basic_value(TypePtr type);

    template<typename T>
    VarPtr create_basic_value()
    {
        return create_basic_value(primitive_t{T{}});
    }

    ValuePtr copy_value_from_port(const mx::PortElementPtr& port);
}

#endif //MXSLC_SERIALIZER_UTILS_H
