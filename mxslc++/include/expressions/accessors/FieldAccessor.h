//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_FIELDACCESSOR_H
#define MXSLC_FIELDACCESSOR_H

#include <variant>

#include "expressions/accessors/Accessor.h"
#include "common.h"

namespace mxslc::expressions
{
    class FieldAccessor final : public Accessor
    {
    public:
        FieldAccessor(ExprPtr value_expr, int field_index);
        FieldAccessor(vector<TypePtr> target_types, ExprPtr value_expr, string field_name);

        TypePtr type() const override;
        VarPtr evaluate() const override;

    private:
        vector<TypePtr> target_types_;
        ExprPtr value_expr_;
        std::variant<int, string> field_;
    };
}

#endif //MXSLC_FIELDACCESSOR_H
