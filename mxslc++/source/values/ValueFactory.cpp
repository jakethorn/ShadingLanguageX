//
// Created by jaket on 12/01/2026.
//

#include "ValueFactory.h"
#include "runtime/Parameter.h"
#include "InterfaceValue.h"
#include "UnnamedStructValue.h"
#include "mtlx/mtlx_utils.h"

ValuePtr ValueFactory::create_parameter_interface(const Parameter& param)
{
    if (param.type().is_complex())
    {
        vector<ValuePtr> subvals;
        for (size_t i = 0; i < param.type().subtype_count(); ++i)
        {
            ValuePtr subval = std::make_unique<InterfaceValue>(port_name(param.name(), i), param.type().subtype(i));
            subvals.push_back(std::move(subval));
        }

        return std::make_unique<UnnamedStructValue>(std::move(subvals));
    }
    else
    {
        return std::make_unique<InterfaceValue>(param.name(), param.type());
    }
}
