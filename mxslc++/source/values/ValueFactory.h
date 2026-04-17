//
// Created by jaket on 12/01/2026.
//

#ifndef MXSLC_VALUEFACTORY_H
#define MXSLC_VALUEFACTORY_H

#include <MaterialXCore/Node.h>

#include "BasicValue.h"
#include "utils/common.h"

class Parameter;

class ValueFactory
{
public:
    static VarPtr2 create_interface_value(TypeInfoPtr type, const string& name);
    static VarPtr2 create_interface_value(const Parameter& param);
    static ValuePtr create_node_value(mx::NodePtr node, TypeInfoPtr type);
    static ValuePtr create_output_value(mx::NodePtr node, const string& output_name, TypeInfoPtr type);
    static ValuePtr create_default_value(TypeInfoPtr type);
    static ValuePtr copy_value_from_port(const mx::PortElementPtr& port);
    static ValuePtr cast_value(ValuePtr value, TypeInfoPtr type);
    static ValuePtr create_empty_value(TypeInfoPtr type);

    template<typename T>
    static ValuePtr create_default_value()
    {
        return std::make_shared<BasicValue>(T{});
    }

private:
    ValueFactory() = default;
    ~ValueFactory() = default;
};

#endif //MXSLC_VALUEFACTORY_H
