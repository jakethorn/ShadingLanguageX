//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_ACCESSOR_H
#define MXSLC_ACCESSOR_H

#include "utils/common.h"

class Accessor
{
public:
    Accessor() = default;
    virtual ~Accessor() = default;

    virtual TypePtr type() const = 0;
    virtual VarPtr evaluate() const = 0;
};

#endif //MXSLC_ACCESSOR_H
