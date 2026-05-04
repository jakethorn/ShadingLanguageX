//
// Created by jaket on 04/05/2026.
//

#include "FieldAccessor.h"

#include "expressions/Expression.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

FieldAccessor::FieldAccessor(VarPtr var, string property) : var_{std::move(var)}, property_{std::move(property)}
{

}

TypePtr FieldAccessor::type() const
{
    return var_->type()->field_type(property_);
}

VarPtr FieldAccessor::evaluate() const
{
    return var_->child(property_);
}
