//
// Created by jaket on 01/12/2025.
//

#include "ParameterList.h"

#include "Argument.h"

const Parameter& ParameterList::operator[](const Argument& a) const
{
    if (a.has_name())
        return (*this)[a.name()];
    return (*this)[a.index()];
}

void ParameterList::init() const
{
    for (const Parameter& param : params_)
        param.init();
}
