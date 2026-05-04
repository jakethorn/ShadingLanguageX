//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_FIELDACCESSOR_H
#define MXSLC_FIELDACCESSOR_H

#include "DotAccessor.h"
#include "utils/common.h"

class FieldAccessor final : public DotAccessor
{
public:
    FieldAccessor(VarPtr var, string property);

    TypePtr type() const override;
    VarPtr evaluate() const override;

private:
    VarPtr var_;
    string property_;
};

#endif //MXSLC_FIELDACCESSOR_H
