//
// Created by jaket on 12/01/2026.
//

#ifndef MXSLC_VALUEFACTORY_H
#define MXSLC_VALUEFACTORY_H

#include <MaterialXCore/Node.h>

#include "common.h"
#include "primitive_t.h"

namespace mxslc::runtime
{
    class AttributeList;
}

namespace mxslc::values
{
    class ValueFactory
    {
    public:
        static VarPtr create_interface_value(TypePtr type, const string& name);
        static VarPtr create_node_value(mx::NodePtr node, const mx::NodeDefPtr& node_def, TypePtr type);
        static VarPtr create_node_value(mx::NodePtr node, const FuncPtr& func);
        static VarPtr create_node_graph_value(const mx::NodeGraphPtr& node_graph, TypePtr type);
        static VarPtr create_node_graph_value(const FuncPtr& func);
        static VarPtr create_output_value(mx::NodePtr node, TypePtr type, const string& output_name);
        static VarPtr create_output_value(mx::NodePtr node, TypePtr type, const string& output_name, const AttributeList& attrs);
        static VarPtr create_output_values(mx::NodePtr node, TypePtr type, const vector<string>& output_names);
        static VarPtr create_output_value(mx::NodeGraphPtr node_graph, TypePtr type, const string& output_name);
        static VarPtr create_default_value(TypePtr type);
        static ValuePtr copy_value_from_port(const mx::PortElementPtr& port);

        template<typename T>
        static VarPtr create_default_value()
        {
            return create_default_value_impl(T{});
        }

    private:
        ValueFactory() = default;
        ~ValueFactory() = default;

        static VarPtr create_default_value_impl(primitive_t value);
    };
}

#endif //MXSLC_VALUEFACTORY_H
