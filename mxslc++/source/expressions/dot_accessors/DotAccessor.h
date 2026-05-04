//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_DOTACCESSOR_H
#define MXSLC_DOTACCESSOR_H

#include "utils/common.h"

class DotAccessor
{
public:
    DotAccessor() = default;
    virtual ~DotAccessor() = default;

    virtual TypePtr type() const = 0;
    virtual VarPtr evaluate() const = 0;
};

#endif //MXSLC_DOTACCESSOR_H
