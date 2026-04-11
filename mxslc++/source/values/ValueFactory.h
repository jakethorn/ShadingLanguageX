//
// Created by jaket on 12/01/2026.
//

#ifndef MXSLC_VALUEFACTORY_H
#define MXSLC_VALUEFACTORY_H

#include <MaterialXCore/Node.h>

#include "utils/common.h"

class Parameter;

class ValueFactory
{
public:
    static ValuePtr create_interface_value(const TypeInfoPtr& type, const string& name);
    static ValuePtr create_interface_value(const Parameter& param);
    static ValuePtr create_interface_value(const VarPtr& var);
    static ValuePtr create_node_value(mx::NodePtr node, TypeInfoPtr type);
    static ValuePtr create_output_value(mx::NodePtr node, const string& output_name, TypeInfoPtr type);
    static ValuePtr cast_value(ValuePtr value, TypeInfoPtr type);

private:
    ValueFactory() = default;
    ~ValueFactory() = default;
};

#endif //MXSLC_VALUEFACTORY_H
