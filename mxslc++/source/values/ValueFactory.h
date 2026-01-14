//
// Created by jaket on 12/01/2026.
//

#ifndef MXSLC_VALUEFACTORY_H
#define MXSLC_VALUEFACTORY_H

#include "utils/common.h"

class Parameter;

class ValueFactory
{
public:
    static ValuePtr create_parameter_interface(const Parameter& param);

private:
    ValueFactory() = default;
    ~ValueFactory() = default;
};

#endif //MXSLC_VALUEFACTORY_H
