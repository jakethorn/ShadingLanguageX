//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_FIELDACCESSOR_H
#define MXSLC_FIELDACCESSOR_H

#include "Accessor.h"
#include "utils/common.h"

class FieldAccessor final : public Accessor
{
public:
    FieldAccessor(VarPtr var, const string& property);
    FieldAccessor(VarPtr var, size_t index);

    TypePtr type() const override;
    VarPtr evaluate() const override;

private:
    VarPtr var_;
    size_t index_;
};

#endif //MXSLC_FIELDACCESSOR_H
