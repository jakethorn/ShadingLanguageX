//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_FIELDACCESSOR_H
#define MXSLC_FIELDACCESSOR_H

#include "expressions/accessors/Accessor.h"
#include "common.h"

namespace mxslc::expressions
{
    class FieldAccessor final : public Accessor
    {
    public:
        FieldAccessor(const vector<TypePtr>& types, VarPtr var, const string& property);
        FieldAccessor(VarPtr var, int index);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        VarPtr var_;
        VarPtr field_var_;
    };
}

#endif //MXSLC_FIELDACCESSOR_H
