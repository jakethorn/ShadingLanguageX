//
// Created by jaket on 12/01/2026.
//

#ifndef MXSLC_VALUEFACTORY_H
#define MXSLC_VALUEFACTORY_H

#include <MaterialXCore/Node.h>

#include "BasicValue.h"
#include "runtime/Variable2.h"
#include "utils/common.h"

class Parameter;

class ValueFactory
{
public:
    static VarPtr2 create_interface_value(TypeInfoPtr type, const string& name);
    static VarPtr2 create_node_value(mx::NodePtr node, const mx::NodeDefPtr& node_def, TypeInfoPtr type);
    static VarPtr2 create_node_value(mx::NodePtr node, const FuncPtr2& func);
    static VarPtr2 create_output_value(mx::NodePtr node, TypeInfoPtr type, const string& output_name);
    static VarPtr2 create_output_values(mx::NodePtr node, TypeInfoPtr type, const vector<string>& output_names);
    static VarPtr2 create_default_value(TypeInfoPtr type);
    static ValuePtr copy_value_from_port(const mx::PortElementPtr& port);
    //static ValuePtr cast_value(ValuePtr value, TypeInfoPtr type);
    //static ValuePtr create_empty_value(TypeInfoPtr type);

    template<typename T>
    static VarPtr2 create_default_value()
    {
        ValuePtr value = std::make_shared<BasicValue>(T{});
        return Variable2::create(std::move(value));
    }

private:
    ValueFactory() = default;
    ~ValueFactory() = default;
};

#endif //MXSLC_VALUEFACTORY_H
