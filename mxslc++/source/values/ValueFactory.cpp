//
// Created by jaket on 12/01/2026.
//

#include "ValueFactory.h"
#include "runtime/Parameter.h"
#include "InterfaceValue.h"
#include "UnnamedStructValue.h"
#include "mtlx/mtlx_utils.h"

namespace
{
    ValuePtr get_interface_from_type(const TypeInfoPtr& type, const string& name)
    {
        if (type->has_fields())
        {
            vector<ValuePtr> field_vals;
            for (size_t i = 0; i < type->field_count(); ++i)
            {
                ValuePtr field_val = get_interface_from_type(type->field_type(i), port_name(name, i));
                field_vals.push_back(std::move(field_val));
            }

            return std::make_shared<UnnamedStructValue>(std::move(field_vals));
        }
        else
        {
            return std::make_shared<InterfaceValue>(name, type);
        }
    }
}

ValuePtr ValueFactory::create_parameter_interface(const Parameter& param)
{
    return get_interface_from_type(param.type(), param.name());
}
