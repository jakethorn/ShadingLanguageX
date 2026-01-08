//
// Created by jaket on 03/01/2026.
//

#include "ArgumentList.h"
#include "Parameter.h"

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
