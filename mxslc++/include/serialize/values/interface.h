//
// Created by jaket on 08/07/2026.
//

#ifndef MXSLC_VALUES_INTERFACE_H
#define MXSLC_VALUES_INTERFACE_H

#include <MaterialXCore/Node.h>

#include "common.h"
#include "primitive_t.h"

namespace mxslc::runtime
{
    class AttributeList;
}

namespace mxslc::values
{
    template<typename T, typename... Args>
    shared_ptr<T> create_value(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    ValuePtr create_value(primitive_t value);

    template<typename T>
    shared_ptr<T> cast_value(const ValuePtr& value)
    {
        return std::dynamic_pointer_cast<T>(value);
    }

#define TYPE_DEF(T) class T; \
using T##Ptr = shared_ptr<T>;

    TYPE_DEF(BasicValue)
    TYPE_DEF(InterfaceValue)
    TYPE_DEF(NodeGraphOutputValue)
    TYPE_DEF(NodeGraphValue)
    TYPE_DEF(NodeOutputValue)
    TYPE_DEF(NodeOutputGraphValue)
    TYPE_DEF(NodeValue)

    namespace value_utils
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
        VarPtr create_default_value(TypePtr type);

        template<typename T>
        VarPtr create_default_value()
        {
            return create_basic_value(primitive_t{T{}});
        }

        ValuePtr copy_value_from_port(const mx::PortElementPtr& port);
    }
}

#endif //MXSLC_VALUES_INTERFACE_H
