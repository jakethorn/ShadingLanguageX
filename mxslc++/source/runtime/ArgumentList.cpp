//
// Created by jaket on 03/01/2026.
//

#include "ArgumentList.h"
#include "Parameter.h"

VarPtr2 ArgumentList::evaluate(const Parameter& param) const
{
    VarPtr2 value;
    if (const Argument* arg = (*this)[param])
        value = arg->evaluate();
    else if (param.has_default_value())
        value = param.evaluate();
    else
        throw CompileError{"Function call missing argument for parameter '" + param.name() + "'"};
    return value;
}

const Argument* ArgumentList::operator[](const Parameter& param) const
{
    for (const Argument& arg : args_)
    {
        if (arg.has_name())
        {
            if (arg.name() == param.name())
                return &arg;
        }
        else
        {
            if (arg.index() == param.index())
                return &arg;
        }
    }
    return nullptr;
}
