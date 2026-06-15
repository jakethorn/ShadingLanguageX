//
// Created by jaket on 04/05/2026.
//

#include "FieldAccessor.h"

#include "runtime/Type.h"
#include "runtime/Variable.h"

FieldAccessor::FieldAccessor(VarPtr var, const string& property) : var_{std::move(var)}
{
    index_ = var_->type()->field_index(property);
}

FieldAccessor::FieldAccessor(VarPtr var, const int index) : var_{std::move(var)}
{
    if (index >= 0)
        index_ = index;
    else
        index_ = var_->type()->field_count() + index;
}

TypePtr FieldAccessor::type() const
{
    return var_->type()->field_type(index_);
}

VarPtr FieldAccessor::evaluate() const
{
    return var_->child(index_);
}
