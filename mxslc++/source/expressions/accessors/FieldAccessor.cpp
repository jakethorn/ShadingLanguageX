//
// Created by jaket on 04/05/2026.
//

#include "expressions/accessors/FieldAccessor.h"

#include "runtime/FunctionQuery.h"
#include "runtime/utils/invoke.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

namespace mxslc::expressions
{
    FieldAccessor::FieldAccessor(const vector<TypePtr>& types, VarPtr var, const string& property) : var_{std::move(var)}
    {
        if (var_->type()->has_field(property))
            field_var_ = var_->child(property);

        if (scope().has_function({var_->type(), types, property, /*is_parameterless*/true}))
            field_var_ = runtime_utils::invoke_method(types, var_, property);

        if (field_var_ == nullptr)
            throw CompileError{"Expression of type " + var_->type()->str() + " does not have a field or parameterless method with the name " + property};
    }

    FieldAccessor::FieldAccessor(VarPtr var, int index) : var_{std::move(var)}
    {
        if (index < 0)
            index = static_cast<int>(var_->type()->field_count()) + index;
        field_var_ = var_->child(index);
    }

    TypePtr FieldAccessor::type() const
    {
        return field_var_->type();
    }

    VarPtr FieldAccessor::evaluate() const
    {
        return field_var_;
    }
}
